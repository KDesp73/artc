#!/usr/bin/env python3

import os
import sys
import subprocess
import datetime
import requests
from dotenv import load_dotenv
from atproto import Client, models, client_utils
import json

load_dotenv()

# Config
BLUESKY_HANDLE = os.getenv("BLUESKY_HANDLE")
BLUESKY_PASSWORD = os.getenv("BLUESKY_PASSWORD")

def generate_description(lua_script):
    return client_utils.TextBuilder().text(f"{lua_script}\n\nMade with ").tag("#artc", "artc")

def get_video_dimensions(filepath):
    result = subprocess.run([
        "ffprobe", "-v", "error", "-select_streams", "v:0",
        "-show_entries", "stream=width,height",
        "-of", "json", filepath
    ], capture_output=True, text=True, check=True)

    info = json.loads(result.stdout)
    width = info["streams"][0]["width"]
    height = info["streams"][0]["height"]
    return width, height

def main() -> None:
    if len(sys.argv) < 2:
        print("Usage: upload.py <video>")
        sys.exit(1)

    video = sys.argv[1]
    script = f"{os.path.basename(video).split(".")[0]}.lua"

    description = generate_description(script)

    width, height = get_video_dimensions(video)
    aspect_ratio = models.AppBskyEmbedDefs.AspectRatio(width=width, height=height)

    client = Client()
    client.login(BLUESKY_HANDLE, BLUESKY_PASSWORD)

    with open(video, 'rb') as f:
        vid_data = f.read()

    client.send_video(
        text=description,
        video=vid_data,
        video_alt=script,
        video_aspect_ratio=aspect_ratio,
    )

    print("✅ Posted successfully!")

if __name__ == "__main__":
    main()
