#!/usr/bin/env python3
'''This script searches specified base directories for subdirectories containing a .yml file,
and runs Ceedling (a C unit testing framework) in each found directory. It is intended to
automate the discovery and execution of unit tests for embedded software projects using Ceedling.
Features:
- Recursively searches for directories with .yml files, indicating Ceedling project roots.
- Runs Ceedling's 'gcov:all' command in each discovered directory using Ruby.
- Prints a summary of all directories where Ceedling was executed.
Usage:
- Modify the 'base_directories' list to specify which directories to search.
- Run the script directly to execute Ceedling in all found test directories.
Dependencies:
- Python 3.x
- Ruby (for running Ceedling, which is present in the current repository)
Note:
- This script is intended for use in a development environment with Ceedling-based C unit tests.
'''

import os  # Import the os module to interact with the operating system
import subprocess  # Import the subprocess module to run external commands
import argparse # Import argparse for command-line argument parsing
import shutil  # For copying directories

# Set the relative path to the Ceedling executable
CEEDLING_CMD = os.path.join("unit_tests", "vendor", "ceedling", "bin", "ceedling")

def copy_build_to_runnable_all_tests(test_root, script_dir):
    """
    Copy the 'build' folder from the test_root to the 'runnableAllTests' folder in script_dir,
    renaming it to match the name of the test_root folder.
    """
    build_src = os.path.join(test_root, "build/artifacts/gcov")
    if not os.path.exists(build_src):
        print(f"No 'build' folder found in {test_root}, skipping copy.")
        return
    dest_dir = os.path.join(script_dir, "TestReportCollection")
    os.makedirs(dest_dir, exist_ok=True)
    test_folder_name = os.path.basename(os.path.normpath(test_root))
    build_dst = os.path.join(dest_dir, test_folder_name)
    # Remove destination if it already exists
    if os.path.exists(build_dst):
        shutil.rmtree(build_dst)
    shutil.move(build_src, build_dst)
    print(f"Copied build folder from {test_root} to {build_dst}")

def find_and_run_ceedling(base_dirs, script_dir):
    """
    Search for directories containing a .yml file and run Ceedling in those directories.
    Returns a list of folders where Ceedling was run.
    """
    found_folders = []
    for base_dir in base_dirs:
        abs_base_dir = os.path.abspath(base_dir)
        for root, dirs, files in os.walk(abs_base_dir):
            if any(file.endswith(".yml") for file in files):
                print(f"Found .yml in: {root} → Running Ceedling")
                found_folders.append(root)
                ceedling_cmd = os.path.join(script_dir, CEEDLING_CMD)
                print(f"Running command: ruby {ceedling_cmd} in {root}")
                try:
                    subprocess.run(["ruby", ceedling_cmd, "gcov:all"], cwd=root, check=True)
                    copy_build_to_runnable_all_tests(root, script_dir)
                except subprocess.CalledProcessError as e:
                    print(f"Error running Ceedling in {root}: {e}")
                    copy_build_to_runnable_all_tests(root, script_dir)
                    # After copying, rename the copied directory to append "_failed"
                    dest_dir = os.path.join(script_dir, "TestReportCollection")
                    test_folder_name = os.path.basename(os.path.normpath(root))
                    build_dst = os.path.join(dest_dir, test_folder_name)
                    build_dst_failed = build_dst + "_failed"
                    if os.path.exists(build_dst):
                        if os.path.exists(build_dst_failed):
                            shutil.rmtree(build_dst_failed)
                        os.rename(build_dst, build_dst_failed)
                        print(f"Renamed build folder to {build_dst_failed}")
                dirs.clear()  # Yaml has been found, no need to search deeper
    return found_folders


def print_found_folders(folders):
    """Print the list of folders where Ceedling was run."""
    if folders:
        print("\nCeedling was run in the following folders:")
        for folder in folders:
            print(f"  - {folder}")
    else:
        print("No folders found containing a .yml file.")


def main():
    """Main entry point for the script."""
    # TODO: use --collection to collect tests in one location and run them all together for a comprehensive report.
    # parser = argparse.ArgumentParser(description="Run Ceedling or collect test folders.")
    # parser.add_argument('--collection', action='store_true', help='Only collect and print test folders, do not run Ceedling')
    # args = parser.parse_args()

    current_file_dir = os.path.dirname(os.path.abspath(__file__))
    # You can modify this list to add more directories to search
    base_directories = [
       "../product/lin_drv/test/TEST_ApplLinDiagEcuReset",
       "../product/lin_drv/test/TEST_ApplLinDiagReadDataByAddress",
       "../product/lin_drv/test/TEST_WriteTxMsg1",
       "../product/lin_drv/test/TEST_ReadRxMsg1",
       "../product/lin_drv/test/TEST_updateTargetSpeed",
       "../product/lin_drv/test/TEST_updatePostRunSpeed",
       "../product/lin_drv/test/TEST_updateLINCommFailFault",
       "../product/lin_drv/test/TEST_RdbiVhitActualStatusDiag_",
       "../product/lin_drv/test/TEST_LinApplMon_Run",
       "../product/system_state/unit_tests/TEST_UpdateTransition_",
       "../product/system_state/unit_tests/TEST_Postrun_handler",
       "../product/system_state/unit_tests/TEST_Sensorless_handler",
       "../platform/basic_sw_platform/system_state/unit_tests/TEST_SysState_run",
       "../platform/basic_sw_platform/bridge_ctrl/unit_tests/TEST_InitStateHandler",
       "../platform/basic_sw_platform/bridge_ctrl/unit_tests/TEST_OffStateHandler",
       "../platform/basic_sw_platform/bridge_ctrl/unit_tests/TEST_OnStateHandler",
       "../platform/basic_sw_platform/bridge_ctrl/unit_tests/TEST_FaultStateHandler",
       "../platform/basic_sw_platform/bridge_ctrl/unit_tests/TEST_ChargePumpVoltageMonitoring_b",
       "../platform/basic_sw_platform/hw_rev_identifier/unit_tests/TEST_HwRevDetection",
       "../platform/basic_sw_platform/hal/unit_tests/TEST_mcal",
       "../platform/basic_sw_platform/monitoring/test/TEST_SpeedMon_Run",
       "../platform/basic_sw_platform/monitoring/test/TEST_VoltMon_Run",
       "../platform/basic_sw_platform/monitoring/test/TEST_VoltMon_permFault_Run",
       "../platform/basic_sw_platform/measurement/test/TEST_VoltMeas_Run",
       "../platform/basic_sw_platform/error_collector/test/TEST_EC_Set_error_shunt_fault_State_e",
       "../platform/mot_ctrl_wrapper/unit_tests/TEST_ForceMotorStop",
       "../platform/mot_ctrl_wrapper/unit_tests/TEST_MotCtrl_run_SR_task",
       #"../platform",
        ]
    found_folders = find_and_run_ceedling(base_directories, current_file_dir)
    print_found_folders(found_folders)

if __name__ == "__main__":
    main()
