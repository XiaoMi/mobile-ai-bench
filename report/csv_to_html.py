import os
import pandas as pd

csvs = ['run_report', 'prepare_report', 'precision_report']

for csv in csvs:
    csv_file = 'output/' + csv + '.csv'
    if os.path.exists(csv_file):
        df = pd.read_csv('output/' + csv + '.csv')
        df.to_html('report/' + csv + '.html')

dir_path = os.path.dirname(os.path.realpath(__file__))
print("Open %s/index.html in a browser to see the report!" % dir_path)
