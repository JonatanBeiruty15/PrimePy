# primepy/core/__init__.py

import os
import sys

# Get the directory where the current __init__.py file is located
current_dir = os.path.dirname(os.path.abspath(__file__))

# Add this directory to the Python path
sys.path.insert(0, current_dir)

# Now, import the _core module, which should be in the same directory
try:
    import _core
except ImportError as e:
    raise ImportError(f"Could not import the native _core module: {e}")

# Clean up the path to avoid unintended side effects
sys.path.remove(current_dir)