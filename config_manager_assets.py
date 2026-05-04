import os
import gzip
import shutil

try:
    Import("env") # type: ignore
except NameError:
    from SCons.Script import env # type: ignore

# --- PHASE 1: FORCE DIRECTORY RECOGNITION ---
# We fetch the path directly from the project configuration
project_config = env.GetProjectConfig()
project_dir = env.get("PROJECT_DIR")
data_dir_name = project_config.get("env:" + env["PIOENV"], "data_dir", "data")
full_data_path = os.path.join(project_dir, data_dir_name)

if not os.path.exists(full_data_path):
    print(f"--- ConfigManager: Pre-creating data directory: {full_data_path} ---")
    os.makedirs(full_data_path)

def sync_and_compress_assets(source, target, env):
    # 1. Setup Paths
    # Path to your library's private assets
    lib_dir = os.path.dirname(os.path.realpath(__file__))
    lib_data_src = os.path.join(lib_dir, "data")

    # Target subdirectory to isolate your library files
    # This prevents overwriting the user's own index.html
    target_sub_dir = os.path.join(full_data_path, "cm")

    # 2. Sync Phase: Copy library files to the project
    if os.path.exists(lib_data_src):
        print(f"--- ConfigManager: Syncing isolated assets to {target_sub_dir} ---")
        if not os.path.exists(target_sub_dir):
            os.makedirs(target_sub_dir)
        
        for item in os.listdir(lib_data_src):
            s = os.path.join(lib_data_src, item)
            d = os.path.join(target_sub_dir, item)
            if not os.path.isdir(s): # Only copy files
                shutil.copy2(s, d)

    # 3. Compression Phase: Gzip the files in the project data dir
    # We scan the WHOLE project data dir so user files get compressed too (a nice bonus!)
    print(f"--- ConfigManager: Auto-compressing all assets in {full_data_path} ---")
    for root, dirs, files in os.walk(full_data_path):
        for file in files:
            # Compress common web files that don't have a .gz version yet
            if file.endswith((".html", ".css", ".js", ".json")) and not file.endswith(".gz"):
                file_path = os.path.join(root, file)
                gz_path = file_path + ".gz"
                
                # Only compress if .gz doesn't exist or source is newer
                if not os.path.exists(gz_path) or os.stat(file_path).st_mtime > os.stat(gz_path).st_mtime:
                    with open(file_path, 'rb') as f_in:
                        with gzip.open(gz_path, 'wb') as f_out:
                            shutil.copyfileobj(f_in, f_out)
                    print(f"  + Compressed: {os.path.relpath(gz_path, full_data_path)}")

env.AddPreAction("buildfs", sync_and_compress_assets)
env.AddPreAction("uploadfs", sync_and_compress_assets)