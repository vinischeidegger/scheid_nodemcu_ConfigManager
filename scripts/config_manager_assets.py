import os
import gzip
import shutil
import re

try:
    Import("env") # type: ignore
except NameError:
    from SCons.Script import env # type: ignore

# Optional minification support
try:
    from css_html_js_minify import html_minify
    MINIFIER_AVAILABLE = True
except ImportError:
    MINIFIER_AVAILABLE = False

# --- PHASE 1: FORCE DIRECTORY RECOGNITION ---
# We fetch the path directly from the project configuration
project_config = env.GetProjectConfig()
project_dir = env.get("PROJECT_DIR")
data_dir_name = project_config.get("env:" + env["PIOENV"], "data_dir", "data")
full_data_path = os.path.join(project_dir, data_dir_name)

print(f"DEBUG: ConfigManager Script Loaded. Target: {full_data_path}")

if not os.path.exists(full_data_path):
    print(f"--- ConfigManager: Pre-creating data directory: {full_data_path} ---")
    os.makedirs(full_data_path)

def sync_and_compress_assets(source, target, env):
    # 1. Setup Paths
    # Path to your library's private assets
    try:
        current_script_path = __file__
    except NameError:
        # If __file__ is missing, we use the env to get the script's path
        # or assume the library root based on the script's known location
        import inspect
        current_script_path = inspect.getfile(lambda: None)

    lib_dir = os.path.dirname(os.path.realpath(current_script_path))
    
    # If your script is inside a 'scripts' folder, go up one level to find 'data'
    if lib_dir.endswith('scripts'):
        lib_dir = os.path.dirname(lib_dir)

    # Look for source files in the library's web_src folder
    lib_data_src = os.path.join(lib_dir, "web_src")

    # Target subdirectory to isolate your library files
    # This prevents overwriting the user's own index.html
    target_sub_dir = os.path.join(full_data_path, "cm")

    print(f"DEBUG: Syncing from {lib_data_src} to {target_sub_dir}")

    # Check if minification is enabled
    try:
        minify_enabled = project_config.getboolean("env:" + env["PIOENV"], "minify_assets", True)
    except TypeError:
        # Older RawConfigParser implementations do not support a fallback arg.
        try:
            minify_enabled = project_config.getboolean("env:" + env["PIOENV"], "minify_assets")
        except Exception:
            minify_enabled = True
    except Exception:
        minify_enabled = True
    
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

    # 3. Optional JavaScript Inlining Phase
    # Inline external JavaScript files referenced via <script src="..."></script>
    print(f"--- ConfigManager: Inlining JavaScript assets in {full_data_path} ---")
    for root, dirs, files in os.walk(full_data_path):
        for file in files:
            if file.endswith(".html"):
                file_path = os.path.join(root, file)
                try:
                    with open(file_path, 'r', encoding='utf-8') as f:
                        html_content = f.read()
                    
                    # Find all external script references
                    script_pattern = r'<script\s+src=(?:"([^"]+)"|([^ >]+))\s*></script>'
                    
                    def inline_script(match):
                        script_src = match.group(1) or match.group(2)
                        if not script_src:
                            return match.group(0)

                        script_path = os.path.join(root, script_src)

                        # Only inline local files (not absolute paths or URLs).
                        if os.path.exists(script_path) and not script_src.startswith(('http://', 'https://', '/')):
                            try:
                                with open(script_path, 'r', encoding='utf-8') as f:
                                    script_content = f.read()
                                print(f"  + Inlined: {os.path.relpath(script_path, full_data_path)}")
                                return f'<script>{script_content}</script>'
                            except Exception as e:
                                print(f"  ! Warning: Failed to inline {script_src}: {e}")
                                return match.group(0)
                        return match.group(0)
                    
                    # Replace external scripts with inline versions
                    inlined_content = re.sub(script_pattern, inline_script, html_content)
                    
                    # Only write if content changed
                    if inlined_content != html_content:
                        with open(file_path, 'w', encoding='utf-8') as f:
                            f.write(inlined_content)
                        print(f"  + Updated: {os.path.relpath(file_path, full_data_path)}")
                except Exception as e:
                    print(f"  ! Warning: Failed to process {file}: {e}")

    # 4. Optional Minification Phase
    if minify_enabled and MINIFIER_AVAILABLE:
        print(f"--- ConfigManager: Minifying web assets in {full_data_path} ---")
        for root, dirs, files in os.walk(full_data_path):
            for file in files:
                if file.endswith(".html") and not file.endswith(".min.html"):
                    file_path = os.path.join(root, file)
                    try:
                        with open(file_path, 'r', encoding='utf-8') as f:
                            original_content = f.read()
                        if '<script' in original_content.lower():
                            print(f"  + Skipped minifying {os.path.relpath(file_path, full_data_path)} because it contains inline script")
                            continue
                        minified_content = html_minify(original_content)
                        with open(file_path, 'w', encoding='utf-8') as f:
                            f.write(minified_content)
                        print(f"  + Minified: {os.path.relpath(file_path, full_data_path)}")
                    except Exception as e:
                        print(f"  ! Warning: Failed to minify {file}: {e}")
    elif minify_enabled and not MINIFIER_AVAILABLE:
        print("--- ConfigManager: WARNING: minify_assets enabled but css-html-js-minify not installed ---")
        print("  Install with: pip install css-html-js-minify")

    # 5. Compression Phase: Gzip the files in the project data dir
    # We scan the WHOLE project data dir so user files get compressed too (a nice bonus!)
    print(f"--- ConfigManager: Auto-compressing index.html in {full_data_path} ---")
    for root, dirs, files in os.walk(full_data_path):
        for file in files:
            # Skip app.js entirely; it must not be copied or gzipped
            if file != "index.html":
                continue
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
                
                # Delete the source file so only the .gz remains for upload
                os.remove(file_path)
                print(f"  + Optimized: {file} removed, kept {file}.gz")

env.AddPreAction("$BUILD_DIR/littlefs.bin", sync_and_compress_assets)
env.AddPreAction("$BUILD_DIR/spiffs.bin", sync_and_compress_assets)
env.AddPreAction("buildfs", sync_and_compress_assets)
env.AddPreAction("uploadfs", sync_and_compress_assets)