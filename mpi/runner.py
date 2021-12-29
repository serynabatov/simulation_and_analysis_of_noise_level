import os
import json

n_regions = 0
n_sensors = 0
with open("environment.json", "r") as env:
    data = json.load(env)

n_regions = data["n_regions"]

for region in data["regions"]:
    n_sensors += region["n_sensors"]





os.system("make")
os.system("mpirun -oversubscribe -np {} sim".format(n_sensors+n_regions+1))