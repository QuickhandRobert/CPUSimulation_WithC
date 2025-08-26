import os

def count_total_lines(directory):
    total_lines = 0
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith('.c') or file.endswith('.h')or file.endswith('.e'):
                try:
                    with open(os.path.join(root, file), 'r', errors='ignore') as f:
                        total_lines += sum(1 for _ in f)
                except Exception as e:
                    print(f"Error reading {file}: {e}")
    print(total_lines)

# Count lines from current directory
count_total_lines('C:\\Users\\Farbo\\Desktop\\Uni_Stuff\\Term_2\\Advanced Programming\\Project (CPU Simulation WIth C)\\CPUSimulation_WithC\\Computer Simulator')
