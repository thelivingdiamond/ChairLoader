# Converts vtable definition from IDA to C++
# Usage:
#   1. Make file with vtable functions.
#   2. Run
#        python convert_vtable.py <your file name> <class name>
#   3. Pray
import sys
import re
import os
#
if len(sys.argv) != 3 and len(sys.argv) != 4:
    print("Usage: convert_vtable <your file name> <class name> <parent class name(optional)>")
    exit(-1)

#
if len(sys.argv) == 3:
    with open(sys.argv[1], 'r') as f:
        data = f.read()
        classDataHalf = ""
        classVtableHalf = ""
        enumOutput = ""
        isEnumMember = False
        extensions = None
        parentClass =""
        # process the vtable data
        vtableData = re.search(r"struct /\*VFT\*/ " + sys.argv[2] + "_vtbl.*\n{\n(.*;\n)*", data)
        if vtableData is not None:
            classVtableHalf = vtableData.group() \
                .replace('struct /*VFT*/', 'class') \
                .replace('void (__fastcall *~', 'virtual ~') \
                .replace('(__fastcall *', '') \
                .replace(');', ') = 0;') \
                .replace(';\n  ', ';\n  virtual ') \
                .replace(f'I{sys.argv[2]} *this, ', '') \
                .replace(f'I{sys.argv[2]} *this', '') \
                .replace(f'{sys.argv[2]} *this, ', '') \
                .replace(f'{sys.argv[2]} *this', '') \
                .replace(')(', '(') \
                .replace('_vtbl', '') \
                .replace('{\n', '{\npublic:\n')
            for line in classVtableHalf.splitlines():
                if "virtual" in line:
                    if "*this" in line:
                        if sys.argv[2] + " *this" not in line:
                            # print(line)
                            classVtableHalf = classVtableHalf.replace(line + "\n", "")
        else:
            print("No VTable Found")

        # process the class Data
        childClass = False
        classData = re.search("struct __cppobj(.* )" + sys.argv[2] + "( :.*\n|\n){\n(.*;\n)*", data)
        if classData is not None:
            extensions = re.search(" :.*", classData.group())
            if vtableData is None:
                classDataHalf = re.sub("struct __cppobj.*" + sys.argv[2] + ".*\n", "class " + sys.argv[2] + "\n", classData.group())
            else:
                classDataHalf = re.sub("struct __cppobj.*" + sys.argv[2] + ".*\n", "", classData.group())
            classDataHalf = re.sub(f"(I{sys.argv[2]}|{sys.argv[2]})" + r"_vtbl \*__vftable /\*VFT\*/;\n", "", classDataHalf)
        else:
            print("Searching for child class")
            classData = re.search("struct __cppobj(.* )(.*)::" + sys.argv[2] + "( :.*\n|\n){\n(.*;\n)*", data)
            if classData is not None:
                print(classData.group())
                print("Parent class: " + classData.group(2))
                parentClass = classData.group(2)
                if vtableData is None:
                    classDataHalf = re.sub("struct __cppobj.*" + sys.argv[2] + ".*\n", "class " + sys.argv[2] + "\n", classData.group())
                else:
                    classDataHalf = re.sub("struct __cppobj.*" + sys.argv[2] + ".*\n", "", classData.group())
                classDataHalf = re.sub(f"(I{sys.argv[2]}|{sys.argv[2]})" + r"_vtbl \*__vftable /\*VFT\*/;\n", "", classDataHalf)
            else:
                print("Class data not found")
elif len(sys.argv) == 4:
    with open(sys.argv[1], 'r') as f:
        data = f.read()
        classDataHalf = ""
        classVtableHalf = ""
        enumOutput = ""
        isEnumMember = False
        extensions = None
        parentClass = sys.argv[3]
        # process the vtable data
        vtableData = re.search(r"struct /\*VFT\*/ " + sys.argv[2] + "_vtbl.*\n{\n(.*;\n)*", data)
        if vtableData is not None:
            classVtableHalf = vtableData.group() \
                .replace('struct /*VFT*/', 'class') \
                .replace('void (__fastcall *~', 'virtual ~') \
                .replace('(__fastcall *', '') \
                .replace(');', ') = 0;') \
                .replace(';\n  ', ';\n  virtual ') \
                .replace(f'I{sys.argv[2]} *this, ', '') \
                .replace(f'I{sys.argv[2]} *this', '') \
                .replace(f'{sys.argv[2]} *this, ', '') \
                .replace(f'{sys.argv[2]} *this', '') \
                .replace(')(', '(') \
                .replace('_vtbl', '') \
                .replace('{\n', '{\npublic:\n')
            for line in classVtableHalf.splitlines():
                if "virtual" in line:
                    if "*this" in line:
                        if sys.argv[2] + " *this" not in line:
                            # print(line)
                            classVtableHalf = classVtableHalf.replace(line + "\n", "")
        else:
            print("No VTable Found")

        # process the class Data
        childClass = False
        classData = re.search("struct __cppobj(.* )" + sys.argv[3] + "::" + sys.argv[2] + "( :.*\n|\n){\n(.*;\n)*", data)
        if classData is not None:
            extensions = re.search(" :.*", classData.group())
            if vtableData is None:
                classDataHalf = re.sub("struct __cppobj.*" + sys.argv[2] + ".*\n", "class " + sys.argv[2] + "\n", classData.group())
            else:
                classDataHalf = re.sub("struct __cppobj.*" + sys.argv[2] + ".*\n", "", classData.group())
            classDataHalf = re.sub(f"(I{sys.argv[2]}|{sys.argv[2]})" + r"_vtbl \*__vftable /\*VFT\*/;\n", "", classDataHalf)
        else:
            print("Class data not found")

# Get extensions from vtable if classData doesn't have them
if extensions is None:
    extensions = re.search(" :.*", classVtableHalf)

# remove brackets for merging
if classDataHalf != "" and classVtableHalf != "":
    classVtableHalf = classVtableHalf.replace("};", "")
    classDataHalf = classDataHalf.replace("{", "")

# merge class data together
printData = classVtableHalf + classDataHalf

# add extensions
if extensions is not None:
    # print(extensions.group())
    printData = re.sub("class " + sys.argv[2], "class " + sys.argv[2] + extensions.group(), printData)

# enum checking
if printData == "" and parentClass == "":
    enumData = re.search("enum " + sys.argv[2] + "( :.*\n|\n){\n(.*,\n)*};", data)
    if enumData is not None:
        print("Enum Found:")
        enumOutput = enumData.group().replace("enum", "enum class")
        # print(enumOutput)
    else:
        print("Enum is either a member of another class or doesn't exist")
        enumData = re.search("enum (.*)::" + sys.argv[2] + "( :.*\n|\n){\n(.*,\n)*};", data)

        if enumData is not None:
            print("Enum Found")
            print(enumData.group())
            parentClass = enumData.group(1)
            print("Parent Class:")
            print(parentClass)
        else:
            print("Enum not found")
            exit(0)
        if parentClass != "":
            if os.path.exists(parentClass + ".h"):
                print("Parent file exists, appending")
                # append into file and write data
                with open(parentClass + ".h", "r") as parentFile:
                    parentFileData = parentFile.read()
                    if enumData.group() not in parentFileData:
                        parentFileData = re.sub("{\npublic:\n", "{\npublic:\n" + enumData.group() + "\n", parentFileData)
                        print(parentFileData)
                    else:
                        print("Enum already defined in parent file")
                        exit(0)
                with open(parentClass +".h", 'w') as parentFileWrite:
                    parentFileWrite.write(parentFileData)
                    exit(0)
            else:
                while True:
                    response = input("Parent class does not exist, auto generate? y/n ")
                    if response == "y":
                        os.system("IDAConvert.py " + sys.argv[1] + " " + parentClass)
                        print("Appending:")
                        # append into file and write data
                        with open(parentClass + ".h", "r") as parentFile:
                            parentFileData = parentFile.read()
                            if enumData.group() not in parentFileData:
                                parentFileData = re.sub("{\npublic:\n", "{\npublic:\n" + enumData.group() + "\n", parentFileData, 1)
                                print(parentFileData)
                            else:
                                print("Enum already defined in parent file")
                                exit(0)
                        with open(parentClass + ".h", 'w') as parentFileWrite:
                            parentFileWrite.write(parentFileData)
                            exit(0)
                        break
                    elif response == "n":
                        response = input("Generate separate header file? y/n  ")
                        if response == "y":
                            enumOutput = enumData.group()
                        break
                    else:
                        print("Error, please input 'y' or 'n'")

if printData != "":
    with open(sys.argv[2] + ".h", 'w') as file:
        if parentClass == "":
            print(printData)
            file.write(printData)
        elif parentClass != "":
            if os.path.exists(parentClass + ".h"):
                print("Parent file exists, appending")
                # append into file and write data
                with open(parentClass + ".h", "r") as parentFile:
                    parentFileData = parentFile.read()
                    if printData not in parentFileData:
                        print(parentClass)
                        parentFileData = re.sub("({\npublic:\n|{\n)", "{\npublic:\n" + printData + "\n\n", parentFileData, 1)
                        print(parentFileData)
                    else:
                        print("Class already defined in parent file")
                        exit(0)
                with open(parentClass +".h", 'w') as parentFileWrite:
                    parentFileWrite.write(parentFileData)
                    exit(0)
            else:
               print("Error, please generate parent class first: " + parentClass)
elif enumOutput != "":
    with open(sys.argv[2] + ".h", 'w') as file:
        print(enumOutput)
        file.write(enumOutput)
