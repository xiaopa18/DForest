def isFloat(x):
    try:
        float(x)
        return True
    except:
        return False

def read_csv(file_name):
    dataset=[]
    f = open(file_name)
    for i in f:
        data=i.split(',')
        while not isFloat(data[-1]):
            data.remove(data[-1])
        for idx in range(len(data)):
            data[idx]=float(data[idx])
        dataset.append(data)
    f.close()
    return dataset