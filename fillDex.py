import os
import subprocess
import re
import sys
import shutil
import zipfile

def compress_folder(folder_path, zip_path):
    with zipfile.ZipFile(zip_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
        for root, dirs, files in os.walk(folder_path):
            for file in files:
                file_path = os.path.join(root, file)
                arcname = os.path.relpath(file_path, folder_path)
                zipf.write(file_path, arcname=arcname)

def exec_cmd(cmd):
    print(cmd)
    subp = subprocess.Popen(cmd, shell=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    for line in iter(subp.stdout.readline, b''):
        try:
            print(line.decode('utf-8').strip())
            # sys.stdout.flush()
        except Exception as e:
            continue
    exit_code = subp.wait()
    return exit_code

def target(u33_dir):
    item_list = []
    for f in os.listdir(u33_dir):
        if(f.endswith(".item")):
            item = re.search(r'item_(\d+)\.item', f)
            item_list.append(item.group(1))
    return item_list


def deduplication(item_path):
    new_lines = []
    print("deduplication", item_path)
    with open(item_path, "r") as item_fd:
        while(True):
            line = item_fd.readline()
            if(not line):
                break
            if(line not in new_lines):
                new_lines.append(line)
    # item_name, _ = os.path.splitext(os.path.basename(item_path))
    with open(item_path, "w") as item_fd:
        for line in new_lines:
            item_fd.write(line)

def fill_dex(u33_dir):
    item_list = target(u33_dir)
    idx = 1
    for _i in item_list:
        deduplication(f"{u33_dir}/item_{_i}.item")
        out_dex = "out/classes{_idx}.dex".format(_idx = str(idx) if idx != 1 else "")
        shutil.copy2(f"{u33_dir}/dex_{_i}.dex", out_dex)
        exec_cmd(f"./dex -m offset -p on -d {out_dex} -i {u33_dir}/item_{_i}.item")
        idx += 1

if __name__ == "__main__":
    
    if(len(sys.argv) != 2):
        print(sys.argv[0], "u33pk_dir")
    else:
        u33pk_dir = sys.argv[1]
        fill_dex(u33pk_dir)
        compress_folder("out", '{_dir}/out.zip'.format(_dir = u33pk_dir))