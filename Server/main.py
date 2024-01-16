import csv
import json
from typing import List
from datetime import datetime

import fastapi
import uvicorn as uvicorn
from fastapi import FastAPI

from starlette.middleware.cors import CORSMiddleware

app = FastAPI()
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Adjust this to restrict origins if needed
    allow_methods=["*"],  # Adjust this to restrict HTTP methods if needed
    allow_headers=["*"],  # Adjust this to restrict headers if needed
)

@app.post("/start-race")
async def start_race():
    memory = await get_memory()
    if memory["in_progress"]:
        raise fastapi.HTTPException(status_code=400,
                                    detail=f"Race number {memory['current_race_number']} is still in progress")

    memory['current_race_number'] += 1
    memory['in_progress'] = True
    await write_memory(memory)
    return {"message": "ok"}

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
    data_row = [datetime.now().isoformat(), reading]
    await add_sensor_data_to_history(data_row)
    return {"message": "ok"}

async def get_sensor_data_history() -> List[List[str]]:
    with open('sensor_data.csv', mode='r') as file:
        csv_reader = csv.reader(file)
        return [row for row in csv_reader]
async def add_sensor_data_to_history(data: List[str]) -> None:
    with open('sensor_data.csv', mode='a', newline='') as file:
        csv_writer = csv.writer(file)
        csv_writer.writerow(data)
async def get_memory() -> dict:
    with open('data.json') as f:
        data = json.load(f)
    return data

async def write_memory(data: dict) -> None:
    with open('data.json', 'w') as f:
        json.dump(data, f)

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=80)

