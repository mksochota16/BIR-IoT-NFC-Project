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

@app.get("/")
async def base():
    return {"message": "ok"}


if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=80)

