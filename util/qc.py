import os

def get_all_item():
    dir_path = "u33pk"
    for _p in os.listdir(dir_path):
        if(_p.endswith(".item")):
            print(_p)

def target_item():
    item_list = []
    for _i in item_list:
        print(_i)
        qc_file(f"u33pk/item_{_i}.item", f"u33pk/item_{_i}.txt")


def qc_file(filename, outname):
    f = open(filename, "r")
    ss = list(f)
 
    new_ss = []
    for s in ss:
        if s not in new_ss:
            new_ss.append(s)
    f.close()
 
    f = open(outname, "w")
    for line in new_ss:
        f.write(line)
    f.close()
 
def fill_dex():
    item_list = []
    for _i in item_list:
        os.system(f"cp u33pk/dex_{_i}.dex u33pk/dex_{_i}_ori.dex")
        os.system(f"./dex u33pk/dex_{_i}_ori.dex u33pk/item_{_i}.txt")

if __name__ == "__main__":
    # qc_file("item_10402896.item", "item_10402896.txt")
    # target_item()
    fill_dex()
