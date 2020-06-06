from distutils.core import setup, Extension
import subprocess

pngConfigOut = subprocess.Popen(['libpng-config','--ldflags'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
stdout, stderr = pngConfigOut.communicate()
ldArgs = stdout.decode("utf-8").rstrip('\n')

module1 = Extension('PiTerm',
                     sources = ['PiTermModule.cpp', 'Console.cpp', 'ConsoleFBRenderer.cpp'],
                     extra_link_args = [ldArgs])

setup (name = 'PiTerm',
       version = '1.0',
       author = 'James Dessart',
       author_email = 'skwirl42@gmail.com',
       url = 'https://skwirl.com',
       description = 'Module to output a custom console to the adafruit PiTFT 3.5"',
       ext_modules = [module1])
