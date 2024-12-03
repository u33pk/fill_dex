import os
import subprocess
import re
import sys

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
    with open(item_path + ".txt", "w") as item_fd:
        for line in new_lines:
            item_fd.write(line)

def fill_dex(u33_dir):
    item_list = target(u33_dir)
    for _i in item_list:
        exec_cmd(f"./dex -m offset -p on -d u33pk/dex_{_i}.dex -i u33pk/item_{_i}.item")

if __name__ == "__main__":
    if(len(sys.argv) != 2):
        print(sys.argv[0], "u33pk dir")
    else:
        fill_dex(sys.argv[1])