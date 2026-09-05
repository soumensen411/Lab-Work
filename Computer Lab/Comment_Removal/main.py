import re  

with open("./python/input.c" , "r") as f:
    code = f.read()
    
    pattern = r"//.*?$|/\*.*?\/"
    
    clean_code = re.sub(pattern,"",code,flags=re.DOTALL|re.MULTILINE)
    
    
with open("output.c",'w') as f:
    f.write(clean_code)
    
print('successfull')