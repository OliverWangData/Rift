import os
import shutil
import stat

# For Google Highway
# Current customized changes:
#
# Removed the all test files under HWY_TEST_FILES in the cmakelist.txt
# As well as target_sources(skeleton_test PRIVATE hwy/examples/skeleton.cc)
# As some tests that prevent compilation were failing on EMU128. 
# Probably better to investigate why but that is not within scope

def build(configs):
    repository_name = "highway"
    library_version = "1.2.0"
    library_name = "Highway"

    include_path = os.path.join(
        configs["paths"]["project_root"], configs["paths"]["thirdparty_modules_dir"], 
        library_name, "Include"
        )
    intermediate_path = os.path.join(
        configs["paths"]["project_root"], configs["paths"]["thirdparty_intermediate_dir"], 
        library_name
        )
    binaries_path = os.path.join(
        configs["paths"]["project_root"], configs["paths"]["thirdparty_binaries_dir"], 
        library_name
        )
    libraries_path = os.path.join(
        configs["paths"]["project_root"], configs["paths"]["thirdparty_libraries_dir"], 
        library_name
        )
    source_path = os.path.join(
        configs["paths"]["project_root"], configs["paths"]["thirdparty_modules_dir"], 
        library_name, repository_name + "-" + library_version
        )
    cmake_path = os.path.join(
        configs["paths"]["project_root"], configs["paths"]["cmake_app_dir"], 
        "cmake-" + configs["cmake"]["version"], "bin", "cmake.exe"
        )
    
    generator = configs["platform_rules"][configs["current_platform"]]["generator"]
    c_compiler = configs["platform_rules"][configs["current_platform"]]["c_compiler"]
    cxx_compiler = configs["platform_rules"][configs["current_platform"]]["cxx_compiler"]

    # Compiler flags
    # See: https://google.github.io/highway/en/master/README.html#quick-start
    
    # Used by gtest. Google Highway seems to be able to set EHs- and EHc-
    # on its own when needed (Though it spams warning D9025)
    flags = ["/EHsc"]

    if cxx_compiler == "cl.exe":
        flags += ["/O2"]
        
    elif cxx_compiler == "g++" or cxx_compiler == "clang++":
        flags += ["-O2"]

    # Cmake configurations
    cmake_options = [
        "-G", generator,
        "-A", "x64",
        "-DCMAKE_C_COMPILER=" + c_compiler,
        "-DCMAKE_CXX_COMPILER=" + cxx_compiler,
        "-DCMAKE_BUILD_TYPE=Release",
        "-DBUILD_SHARED_LIBS=OFF",
        "-DCMAKE_INSTALL_PREFIX=" + configs["paths"]["project_root"],
        "-DCMAKE_INSTALL_INCLUDEDIR=" + include_path,
        "-DCMAKE_INSTALL_BINDIR=" + binaries_path,
        "-DCMAKE_INSTALL_LIBDIR=" + libraries_path,
        "-DCMAKE_CXX_FLAGS=" + "\"" + " ".join(flags) + "\""
    ]
    
	# ---------------------
	# TODO: Insert logic to prevent a rebuild here. For instance, if target platforms haven't changed
	# ---------------------

	# Clears existing Builds, Includes, Intermediates, Binaries, and Libraries from File Directory
    def remove_path(path):
        
        # Changes read-only files on windows to read-write
        def on_rm_error(func, path, exc_info):
            os.chmod( path, stat.S_IWRITE)
            os.unlink(path)
        
        if os.path.isdir(path):
            shutil.rmtree(path, onerror=on_rm_error)
        
    # Changing compiler flags or anything should always result in a rebuild
    # It's a good idea to rebuild every time anyways if the builds aren't happening a lot
    remove_path(include_path)
    os.makedirs(include_path)
    remove_path(intermediate_path)
    os.makedirs(intermediate_path)
    remove_path(binaries_path)
    os.makedirs(binaries_path)
    remove_path(libraries_path)
    os.makedirs(libraries_path)

	# Configuring, building, and installing cmake
    cmake_configure_commands = [cmake_path, " ".join(cmake_options), 
                                "-B", intermediate_path, source_path]
    cmake_build_commands = [cmake_path, "--build", intermediate_path, "--config", "Release", "-j"]
    cmake_install_commands = [cmake_path, "--install", intermediate_path, "--config", "Release"]
    
    # Setting current working directory into the intermediates folder
    # CMake will spit stuff out here
    original_dir = os.getcwd()
    os.chdir(intermediate_path)

    # Configure
    print(f"\n\nConfiguring cmake for {library_name}\n")
    exit_code = os.system(" ".join(cmake_configure_commands))
    if exit_code != 0:
        os.chdir(original_dir)
        return 1
        
    # Build
    print(f"\n\nBuilding cmake for {library_name}\n")
    exit_code = os.system(" ".join(cmake_build_commands))
    if exit_code != 0:
        os.chdir(original_dir)
        return 1
        
        
    # Install
    print(f"\n\nInstalling cmake for {library_name}\n")
    exit_code = os.system(" ".join(cmake_install_commands))
    if exit_code != 0:
        os.chdir(original_dir)  
        return 1
    
    os.chdir(original_dir)
    return 0
    


if __name__ == "__main__":
    print("Build.py is not meant to be called directly.")