import csv
import json
from typing import List, Optional
from datetime import datetime

import fastapi
import uvicorn as uvicorn
from fastapi import FastAPI
from fastapi.middleware.httpsredirect import HTTPSRedirectMiddleware

from starlette.middleware.cors import CORSMiddleware
CERT_FILE = ""
KEY_FILE = ""

app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Adjust this to restrict origins if needed
    allow_methods=["*"],  # Adjust this to restrict HTTP methods if needed
    allow_headers=["*"],  # Adjust this to restrict headers if needed
)
app.add_middleware(HTTPSRedirectMiddleware)
START_SENSOR_NUMBER = 0

@app.get("/race-time")
async def race_time(race_number: Optional[int] = None):
    memory = await get_memory()
    if memory["in_progress"]:
        raise fastapi.HTTPException(status_code=400,
                                    detail=f"Race number {memory['current_race_number']} is still in progress")

    if race_number is None:
        return {"lap_time": memory["lap_time"]}
    else:
        try:
            race_history: List[List[str]] = await get_race_history(race_number)
            lap_time_row = race_history[-1]
            if lap_time_row[0] != "lap_time":
                return {"message": f"Race number {race_number} did not finish"}
            return {"lap_time": lap_time_row[1]}
        except FileNotFoundError:
            raise fastapi.HTTPException(status_code=400,
                                        detail=f"Race number {memory['current_race_number']} does not exist")




@app.post("/start-race")
async def start_race():
    memory = await get_memory()
    if memory["in_progress"]:
        raise fastapi.HTTPException(status_code=400,
                                    detail=f"Race number {memory['current_race_number']} is still in progress")


    memory['current_race_number'] += 1
    memory['in_progress'] = True

    with open(f"race_{memory['current_race_number']}", 'w', newline='') as csv_file:
        csv_writer = csv.writer(csv_file)
        csv_writer.writerow(['timestamp', 'sensor_number'])

    await write_memory(memory)
    return {"race_number":  memory['current_race_number']}

@app.post("/stop-race")
async def stop_race():
    memory = await get_memory()
    if not memory["in_progress"]:
        raise fastapi.HTTPException(status_code=400,
                                    detail=f"No race is in progress")
    memory['in_progress'] = False
    await write_memory(memory)
    return {"message": "ok"}


@app.post("/sensor-reading")
async def sensor_reading(reading: str):
    if not reading.isdigit():
        raise fastapi.HTTPException(status_code=400,
                                    detail=f"Reading must be a number")
    data_row = [datetime.now().isoformat(), reading]

    memory = await get_memory()
    if memory["in_progress"]:
        await add_reading_to_race(data_row)
        race_finished = await handle_race_logic()
        if race_finished:
            return {"message": "ok"}

    await add_sensor_data_to_history(data_row)
    return {"message": "ok"}


@app.post("/set-checkpoint-number")
async def set_checkpoint_number(num_of_checkpoints: int):
    memory = await get_memory()
    if num_of_checkpoints > memory['num_sensors']:
        raise fastapi.HTTPException(status_code=400, detail=f"Number of checkpoints cannot be greater than {memory['num_sensors']}")

    memory['num_checkpoints'] = num_of_checkpoints
    await write_memory(memory)
    return {"message": "ok"}


async def handle_race_logic() -> bool:
    memory = await get_memory()
    if not memory["in_progress"]:
        raise fastapi.HTTPException(status_code=400,
                              detail=f"No race is in progress")

    race_history: List[List[str]] = await get_race_history()
    race_started_flag = False
    current_checkpoint_number = None
    checkpoints = [int(row[1]) for row in race_history[1:]]
    # the race was for sure not completed
    if not checkpoints.count(START_SENSOR_NUMBER) >= 2 or\
            len(checkpoints) < memory['num_checkpoints'] or\
            checkpoints[-1] != START_SENSOR_NUMBER:
        return False # race not finished

    start_timestamp = None
    for row in race_history[1:]:

        # start logic
        if not race_started_flag and int(row[1]) == START_SENSOR_NUMBER:
            race_started_flag = True
            current_checkpoint_number = START_SENSOR_NUMBER
            start_timestamp = datetime.fromisoformat(row[0])
            continue

        if int(row[1]) == current_checkpoint_number + 1:
            current_checkpoint_number += 1
            continue

        # end logic
        if race_started_flag and\
                int(row[1]) == START_SENSOR_NUMBER\
                and current_checkpoint_number == memory['num_checkpoints'] -1:
            memory["lap_time"] = (datetime.fromisoformat(row[0]) - start_timestamp).total_seconds()
            memory['in_progress'] = False
            await add_reading_to_race(["lap_time", memory["lap_time"]])
            await write_memory(memory)
            return True # race finished

    return False # race is not finished, not all checkpoints were passed



async def add_reading_to_race(data: List[str]) -> None:
    memory = await get_memory()
    with open(f"race_{memory['current_race_number']}", 'a', newline='') as csv_file:
        csv_writer = csv.writer(csv_file)
        csv_writer.writerow(data)


async def get_race_history(race_number: Optional[int] = None) -> List[List[str]]:
    memory = await get_memory()
    with open(f"race_{memory['current_race_number']}" if not race_number else f"race_{race_number}", mode='r') as file:
        csv_reader = csv.reader(file)
        return [row for row in csv_reader]


async def get_sensor_data_history() -> List[List[str]]:
    with open('sensor_data_history.csv', mode='r') as file:
        csv_reader = csv.reader(file)
        return [row for row in csv_reader]


async def add_sensor_data_to_history(data: List[str]) -> None:
    with open('sensor_data_history.csv', mode='a', newline='') as file:
        csv_writer = csv.writer(file)
        csv_writer.writerow(data)


async def get_memory() -> dict:
    with open('memory.json') as f:
        data = json.load(f)
    return data


async def write_memory(data: dict) -> None:
    with open('memory.json', 'w') as f:
        json.dump(data, f)

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8890, ssl_keyfile=KEY_FILE, ssl_certfile=CERT_FILE)

