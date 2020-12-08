from . import wrapper

def to_str_vec(str_list):
    vec = wrapper.str_vec(len(str_list))

    for i in range(len(str_list)):
        vec[i] = str_list[i]

    return vec

def to_int_vec(sample):
    vec = wrapper.int_vec(len(sample))

    for i in range(len(sample)):
        vec[i] = int(sample[i])

    return vec
