
def read_item(item_path):
    with open(item_path, "r") as item_fd:
        while(True):
            line = item_fd.readline()
            if(not line):
                break
            