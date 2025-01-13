import os
import csv

def process_file(file_path, replacements):
    with open(file_path, 'r', encoding='utf-8') as file:
        lines = file.readlines()
    
    with open(file_path, 'w', encoding='utf-8') as file:
        for line in lines:
            if line.strip() == "":  # 如果是空行，则跳过
                continue
            
            # 根据replacements字典替换字符
            for old, new in replacements.items():
                line = line.replace(old, new)
            
            # 处理单行长度超过90的情况
            while len(line) > 90:
                split_at = line.rfind(' ', 0, 90)  # 在前90个字符中寻找最后一个空格
                if split_at == -1:  # 如果没有找到空格（即该行的前90个字符无空格）
                    split_at = 90
                
                file.write(line[:split_at].strip() + '\n')
                line = line[split_at:].lstrip()
            
            file.write(line)

def load_replacements(csv_file):
    replacements = {}
    with open(csv_file, mode='r', encoding='utf-8') as infile:
        reader = csv.reader(infile)
        next(reader)  # 跳过标题行
        for row in reader:
            replacements[row[0]] = row[1]
    return replacements

def main():
    replacements = load_replacements('tools/replace.csv')
    directory = 'data/novels'
    
    for filename in os.listdir(directory):
        if filename.endswith(".txt"):
            process_file(os.path.join(directory, filename), replacements)

if __name__ == "__main__":
    main()