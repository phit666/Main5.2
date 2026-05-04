import xml.etree.ElementTree as ET

tree = ET.parse('Main.vcxproj')
root = tree.getroot()

ns = {'ns': 'http://schemas.microsoft.com/developer/msbuild/2003'}

print("LOCAL_SRC_FILES := \\")

for item in root.findall('.//ns:ClCompile', ns):
    path = item.get('Include')  # SAFE access

    if path:  # skip empty ones
        path = path.replace("\\", "/")
        print(f"    src/{path} \\")