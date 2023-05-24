import math

def dist(a,b):
    res = 0
    for i in range(len(a)):
        res += (a[i]-b[i]) * (a[i]-b[i])
    return math.sqrt(res)

def to_string(data):
    res = ''
    for i in data:
        if res != '':
            res += ','
        res += str(i)
    res += '\n'
    return res