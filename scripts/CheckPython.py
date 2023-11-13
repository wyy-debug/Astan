import subprocess
import pkg_resources
import sys

def install(package):
    print(f"Installing {package} module...")
    subprocess.check_call([sys.executable, "-m", "pip", "install", package])

def ValidatePackage(package):
    required = {package}
    installed = {pkg.key for pkg in pkg_resources.working_set}
    missing = required - installed

    if missing:
        install(package)

def ValidatePackages():
    ValidatePackage('requests')
    ValidatePackage('fake-useragent')