import base64
import hexdump

with open("/home/aosp/Project/dex2fill/classes3.dex", "rb") as dex_fd:
    dex_bytes = dex_fd.read(12794744)
print(len(dex_bytes))
fill_data = "DwABAAMAAQAihJEAmAAAACIAEidwEOH1AABxAPAIAAAMAW4g6/UQABoBYwpuIOz1EABuEPr1AAAMACIBriZwIK7zAQBuELjzAQAKAjkCBQBuEMrzAQBuEMXzAQAMATkBAwAOABYCAAAaBAAAIRUSBgFnEhg1VzkARgkBBxoKxQluIMH1qQAMCiGrNYsDACgpIauxi0YICgtuELr1CAAKChJLNboDACgdbhC69QgACgqxum4wx/VoCgwIcRAo9QgACwoxCAoCPQgNABgMWNSATnsBAAAxCAoMPQgEAAeUBKLYBwcBKMduELr1BAAKATWBAwAOACIBEidwEOH1AQBuIOz1AQBuIOz1QQBuEPr1AQAMAXEQDGUBAAwBIgISJ3AQ4fUCAG4g7PUCAG4g7PVCAG4Q+vUCAAwAbjARZQ4BDgAAAAAAlwABAAEB5k2XAQ=="

fill_bytes = base64.b64decode(fill_data)
hexdump.hexdump(fill_bytes)
dex_fill_off = 0x3952A4

with open("./xxx.dex", "wb") as xxx_fd:
    a = xxx_fd.write(dex_bytes[:dex_fill_off])
    print(a)
    a += xxx_fd.write(fill_bytes[0x10:])
    print(a)
    a += xxx_fd.write(dex_bytes[dex_fill_off + len(fill_bytes) - 0x10:])
    print(a)