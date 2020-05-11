import setuptools

with open("README.md", "r") as fh:
    long_description = fh.read()

setuptools.setup(
    name="bud-first-search", # Replace with your own username
    version="0.0.1",
    author="E. Hebrard, L. Jean",
    author_email="hebrard@laas.fr, ljean@laas.fr",
    description="Bud first search is an algorithm to learn optimal decision trees",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://redmine.laas.fr/laas/users/ehebrard/deeplever.git",
    packages=setuptools.find_packages(),
    package_data= {
        "": ["*.so"]
    },
    classifiers=[
        "Programming Language :: Python :: 3",
        "Operating System :: POSIX :: Linux",
    ],
    python_requires='>=3.5',
)