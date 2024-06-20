from flask import Flask, request, jsonify, send_from_directory
import subprocess
import os
import matplotlib.pyplot as plt

app = Flask(__name__, static_url_path='', static_folder='.')

# Function to generate Gantt chart from data string
def generate_gantt_chart(data, algorithm, output_folder='static'):
    try:
        lines = data.split('\n')

        # Remove irrelevant lines and empty lines
        lines = [line.strip() for line in lines if line.strip() and not line.startswith(('Best', 'Average', 'Gantt Chart:', 'Estimated Quantum Time:', 'Round Robin Scheduling'))]

        task_names = []
        start_times = []
        end_times = []

        for line in lines:
            if line.startswith('|'):
                parts = line.split('|')
                if len(parts) >= 3:
                    task_name = parts[1].strip()
                    times = parts[2].split()

                    if len(times) >= 2:
                        start_time = int(times[0])
                        end_time = int(times[1])

                        task_names.append(task_name)
                        start_times.append(start_time)
                        end_times.append(end_time)

        if len(task_names) != len(start_times) or len(task_names) != len(end_times):
            raise ValueError('Mismatched lengths of tasks, start times, and end times')

        fig, ax = plt.subplots(figsize=(10, 6))

        y_labels = list(set(task_names))
        y_labels.sort()
        y_ticks = range(len(y_labels))

        ax.set_yticks(y_ticks)
        ax.set_yticklabels(y_labels)

        for i, task in enumerate(task_names):
            y_index = y_labels.index(task)
            ax.broken_barh([(start_times[i], end_times[i] - start_times[i])], (y_index - 0.4, 0.8))

        ax.set_xlabel('Time')
        ax.set_ylabel('Tasks')
        ax.grid(True)

        plt.title(f'Gantt Chart - {algorithm}')
        output_filename = os.path.join(output_folder, f'gantt_chart_{algorithm.lower()}.png')
        plt.savefig(output_filename)
        plt.close()

        return output_filename
    
    except Exception as e:
        return str(e)

# Flask route for serving index.html
@app.route('/')
def index():
    return send_from_directory('.', 'index.html')

# Flask route for static files
@app.route('/<path:path>')
def static_files(path):
    return send_from_directory('.', path)

# Flask route for handling scheduling request
@app.route('/schedule', methods=['POST'])
def schedule():
    try:
        data = request.json
        processes = data['processes']

        # Create input file for the C++ program
        with open('input.txt', 'w') as f:
            for p in processes:
                f.write(f"{p['pid']} {p['burst_time']} {p['arrival_time']}\n")

        # Compile the C++ program (assuming all algorithms are in one file)
        compile_result = subprocess.run(['g++', 'main.cpp', '-o', 'scheduling'], capture_output=True, text=True)
        
        if compile_result.returncode != 0:
            raise RuntimeError(f'Compilation failed: {compile_result.stderr}')

        # Run the compiled C++ program
        run_result = subprocess.run(['./scheduling'], capture_output=True, text=True)

        if run_result.returncode != 0:
            raise RuntimeError(f'Execution failed: {run_result.stderr}')

        # Read the output file
        with open('output.txt', 'r') as f:
            output = f.read()

        # Prepare responses for each algorithm
        algorithms = ['FCFS', 'SJF', 'Round Robin', 'SRTF']  # Adjust as per your algorithms
        gantt_charts = {}

        for algorithm in algorithms:
            gantt_chart_url = generate_gantt_chart(output, algorithm)
            gantt_charts[algorithm.lower()] = gantt_chart_url

        return jsonify({'result': output, 'gantt_charts': gantt_charts})

    except Exception as e:
        return jsonify({'error': str(e)}), 500

if __name__ == '__main__':
    app.run(debug=True)
