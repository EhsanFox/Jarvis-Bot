import os
from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()

# This function runs after firmware is uploaded
def after_upload(source, target, env):
    print("📂 Uploading filesystem (LittleFS/SPIFFS)...")
    os.system("pio run --target uploadfs")

# Add the hook
env.AddPostAction("upload", after_upload)
