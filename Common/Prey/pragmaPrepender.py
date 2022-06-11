import os
directory = '.\\'
# list comprehension for all files with a .txt extension
txt_files = [
    f
    for f in os.listdir(directory)
    if os.path.isfile(os.path.join(directory, f)) and '.h' in f
]
for txt in txt_files:
    with open(os.path.join(directory, txt), 'r') as src:
        fileData = src.read()
        src.close()
    # writing file to same name with added .dest extension
    with open(os.path.join(directory, txt), 'w') as dest:
        if "#pragma once" not in fileData:
            dest.write("#pragma once\n" + fileData)
        else:
            dest.write(fileData)