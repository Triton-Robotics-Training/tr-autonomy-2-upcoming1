import sys
if sys.prefix == '/usr':
    sys.real_prefix = sys.prefix
    sys.prefix = sys.exec_prefix = '/home/upcoming/Documents/TR-Training 2/TR-Autonomy-2/src/install/spinnyrobot'
