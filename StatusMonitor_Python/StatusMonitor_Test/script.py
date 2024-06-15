import os
import ctypes
import sys
import subprocess
import time
import wmi
import math
import win32gui
import win32con
import re

def is_admin():
    try:
        return ctypes.windll.shell32.IsUserAnAdmin()
    except:
        return False

def run_as_admin():
    if not is_admin():
        script = os.path.abspath(sys.argv[0])
        params = ' '.join([script] + sys.argv[1:])
        ctypes.windll.shell32.ShellExecuteW(None, "runas", sys.executable, params, None, 1)
        sys.exit()

run_as_admin()

user_home = os.path.expanduser("~")
program_path = os.path.join(user_home, "Desktop", "StatusMonitor_Test", "OpenHardwareMonitor_v0.9.6", "OpenHardwareMonitor.exe")

# HardwareMonitor 프로그램 실행 (관리자 권한으로)
subprocess.Popen([program_path], shell=True)

time.sleep(5)

# 프로그램 창 숨기기
hwnd = win32gui.FindWindow(None, "Open Hardware Monitor")
win32gui.ShowWindow(hwnd, win32con.SW_MINIMIZE)

w = wmi.WMI(namespace="root\\OpenHardwareMonitor")

# CPU 및 GPU 온도 센서의 식별자 찾기
cpu_identifier = None
gpu_identifier = None

for sensor in w.Sensor():
    if re.match(r'.*cpu.*temperature.*', sensor.Identifier, re.IGNORECASE):
        cpu_identifier = sensor.Identifier
    elif re.match(r'.*gpu.*temperature.*', sensor.Identifier, re.IGNORECASE):
        gpu_identifier = sensor.Identifier

if cpu_identifier is None or gpu_identifier is None:
    print("Error: CPU or GPU sensor identifier not found.")
    sys.exit(1)

# CPU 및 GPU 온도 가져오기
def get_cpu_temp():
    cpu_temp = None
    if cpu_identifier:
        cpu_temp = math.ceil(w.Sensor(Identifier=cpu_identifier)[0].Value)
    return cpu_temp

def get_gpu_temp():
    gpu_temp = None
    if gpu_identifier:
        gpu_temp = math.ceil(w.Sensor(Identifier=gpu_identifier)[0].Value)
    return gpu_temp

while True:
    cpu_temp = get_cpu_temp()
    gpu_temp = get_gpu_temp()
    
    if cpu_temp is not None and gpu_temp is not None:
        print("CPU Temp:", cpu_temp, "°C")
        print("GPU Temp:", gpu_temp, "°C")
        print()
    else:
        print("Error: CPU or GPU temperature not found.")
    
    time.sleep(1)
