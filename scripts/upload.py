#!/usr/bin/env python3

import os
import sys
import subprocess
import datetime
import requests
from dotenv import load_dotenv
import json

load_dotenv()

# Config
BLUESKY_HANDLE = os.getenv("BLUESKY_HANDLE")
BLUESKY_PASSWORD = os.getenv("BLUESKY_PASSWORD")
ARTC_PATH = "./artc"
VIDEO_FILE = "output.mp4"

def run_artc(lua_script, duration):
    subprocess.run([ARTC_PATH, lua_script, "--duration", str(duration), "-x", "-o", VIDEO_FILE], check=True)

def generate_description(lua_script, duration):
    timestamp = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%d %H:%M UTC")
    return f"🎨 Generated with #artc\nScript: '{lua_script}'\nDuration: {duration}s\nCreated: {timestamp}\n#artc"

def bluesky_login(handle, password):
    resp = requests.post("https://bsky.social/xrpc/com.atproto.server.createSession", json={
        "identifier": handle,
        "password": password
    })
    resp.raise_for_status()
    return resp.json()["accessJwt"]

def upload_blob(token, video_path):
    with open(video_path, "rb") as f:
        headers = {
            "Authorization": f"Bearer {token}",
            "Content-Type": "application/octet-stream"
        }
        resp = requests.post("https://bsky.social/xrpc/com.atproto.repo.uploadBlob", headers=headers, data=f)
        resp.raise_for_status()
        return resp.json()["blob"]

def get_did(token, handle):
    resp = requests.get(
        "https://bsky.social/xrpc/com.atproto.identity.resolveHandle",
        params={"handle": handle},
        headers={"Authorization": f"Bearer {token}"}
    )
    resp.raise_for_status()
    return resp.json()["did"]

def post_to_bluesky(token, handle, description, blob, did):
    headers = {
        "Authorization": f"Bearer {token}",
        "Content-Type": "application/json"
    }

    post_data = {
        "$type": "app.bsky.feed.post",
        "text": description,
        "createdAt": datetime.datetime.now(datetime.timezone.utc).isoformat().replace("+00:00", "Z"),
        "embed": {
            "$type": "app.bsky.embed.video",
            "video": {
                "$type": "app.bsky.embed.blob",
                "blob": {
                    "$type": "blob",
                    "ref": blob["ref"],
                    "mimeType": blob["mimeType"],
                    "size": blob["size"]
                }
            }
        }
    }

    resp = requests.post(
        "https://bsky.social/xrpc/com.atproto.repo.createRecord",
        headers=headers,
        json={
            "repo": handle,
            "collection": "app.bsky.feed.post",
            "record": post_data
        }
    )
    resp.raise_for_status()

def main():
    if len(sys.argv) < 3:
        print("Usage: upload_to_bluesky.py <scene.lua> <duration_seconds>")
        sys.exit(1)

    lua_script = sys.argv[1]
    duration = int(sys.argv[2])

    run_artc(lua_script, duration)
    description = generate_description(lua_script, duration)

    token = bluesky_login(BLUESKY_HANDLE, BLUESKY_PASSWORD)
    did = get_did(token, BLUESKY_HANDLE)
    blob = upload_blob(token, VIDEO_FILE)
    post_to_bluesky(token, BLUESKY_HANDLE, description, blob, did)

    print("✅ Posted successfully!")

if __name__ == "__main__":
    main()
