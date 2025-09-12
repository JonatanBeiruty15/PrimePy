# setup.py
from setuptools import setup, find_packages

setup(
    name='primepie',
    version='0.1.0',
    author='JB15',
    author_email='BlahBlah@Harta.com',
    description='A number theory library with C++ backend and Python interface',
    long_description=open('README.md').read(),
    long_description_content_type='text/markdown',
    url='https://github.com/JonatanBeiruty15/PrimePy',  
    packages=find_packages(),
    include_package_data=True,
    license='MIT',
    classifiers=[
        'Programming Language :: Python :: 3',
        'License :: OSI Approved :: MIT License',
        'Operating System :: OS Independent',
    ],
    python_requires='>=3.8',
)