import os
import gzip
import shutil

try:
    Import("env") # type: ignore
except NameError:
    from SCons.Script import env # type: ignore

def compress_data_folder(source, target, env):
    data_dir = os.path.join(env.get("PROJECT_DIR"), "data")
    if not os.path.exists(data_dir):
        return

    print(f"--- Auto-compressing assets in {data_dir} ---")
    for root, dirs, files in os.walk(data_dir):
        for file in files:
            if file.endswith((".html", ".css", ".js")):
                file_path = os.path.join(root, file)
                with open(file_path, 'rb') as f_in:
                    with gzip.open(file_path + '.gz', 'wb') as f_out:
                        shutil.copyfileobj(f_in, f_out)
                print(f"Compressed: {file} -> {file}.gz")

# Hook into the 'Upload File System' action
env.AddPreAction("uploadfs", compress_data_folder)