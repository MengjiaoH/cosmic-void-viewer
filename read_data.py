import numpy as np 
import json


json_file = "data/real_original.json"
with open(json_file) as f:
    dict = json.load(f)
    



# data_file = 'data/real_original_map.raw'
# to_file = 'data/real_original_map.raw'

# data = np.fromfile(data_file)
# data = np.reshape(data, (24, 24, 151))
# print(np.min(data), np.max(data))
# # data = data.astype(np.float64)
# data.tofile(to_file)
# print(data.shape, data.dtype)