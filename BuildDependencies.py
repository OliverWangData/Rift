import importlib.util
import os
import platform

# ==================================================================================================
# Classes that group similar CMake settings together

# Converts platform.system() to a standardized platform string
standard_platforms = {
    "Windows" : "Windows",
    "Linux" : "Linux",
    "Darwin" : "MacOS"
    }

configs = {

    "current_platform" : standard_platforms[platform.system()],
    "current_arch" : platform.machine(),

    "platform_rules" : {
        "Windows" : {
            "generator" : '"Visual Studio 17 2022"',
            "c_compiler" : "cl.exe",
            "cxx_compiler" : "cl.exe"
            },
        "Linux" : {
            "generator" : '"Unix Makefiles"',
            "c_compiler" : "gcc",
            "cxx_compiler" : "g++"
            },
        "MacOS" : {
            "generator" : '"XCode"',
            "c_compiler" : "clang",
            "cxx_compiler" : "clang++"
            }
        },

    "paths" : {
        "project_root" : os.path.dirname(os.path.abspath(__file__)),
        "cmake_app_dir" : os.path.join("Programs", "CMake"),
        "thirdparty_modules_dir" : os.path.join("Source", "ThirdParty"),
        "thirdparty_libraries_dir" : os.path.join("Libraries", "ThirdParty"),
        "thirdparty_binaries_dir" : os.path.join("Binaries", "ThirdParty"),
        "thirdparty_intermediate_dir" : os.path.join("Intermediate", "ThirdParty"),
        },

    "cmake" : {
        "version" : "3.31.0"
        }

    }

# ==================================================================================================

def main():
    sucessful_build_count = 0
    failed_build_count = 0

    # Loops through third party modules subdirectories
    third_party_modules_root = os.path.join(
        configs["paths"]["project_root"], 
        configs["paths"]["thirdparty_modules_dir"]
        )

    for item in os.listdir(third_party_modules_root):
    
        item_path = os.path.join(third_party_modules_root, item)
        script_path = os.path.join(item_path, "Build.py")
        
        # Checks if there is a Build.py in the subdirectory
        if not os.path.isdir(item_path) or not os.path.isfile(script_path):
            continue;
        
        # Loads the Build.py
        spec = importlib.util.spec_from_file_location(script_path, script_path) # Use the script path as a unique module name
        build_module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(build_module)
        
        # Checks for a "build" function
        if not hasattr(build_module, "build"):
            print(f"Build method not found in {script_path}.")
            continue
           
        # ==========================================================================================
        # Calls Build.py .build(config). 
        # Build.py should be making changes to various values in the config
        # Mostly to add more arguments to the "cmake_options" key.
        return_code = build_module.build(configs)
        # ==========================================================================================
        
        if return_code is None:
            print(f"{script_path} does not return an exit code.")
        elif return_code == 0:
            print("Build done.")
            sucessful_build_count += 1
        else:
            print(f"Build failed with exit code {return_code}")
            failed_build_count += 1
    
    print(f"Building dependencies complete. Successes: {sucessful_build_count}  Failures: {failed_build_count}")



if __name__ == "__main__":
    main()