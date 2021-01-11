#!/bin/python3

import platform
import argparse
from enum import Enum
import sys
import os

THREADS=4

class InstallationTemplate(Enum):
	DebianLinux = 0
	ArchLinux = 1
	UnknownLinux = 2
	AlpineLinux = 3

	def detect():
		system = platform.system().lower()
		release = platform.release().lower()

		print(f'Detecting platform for \'{system}\' -> \'{release}\'')

		if system == 'linux':
			if release.find('alpine'):
				return InstallationTemplate.AlpineLinux
			elif release.find('arch') or release.find('manjaro'):
				return InstallationTemplate.ArchLinux
			elif release.find('ubuntu') or release.find('debian'):
				return InstallationTemplate.DebianLinux
			else:
				print(f'Your current linux distro \'{release}\' is not supported yet')
				return InstallationTemplate.UnknownLinux
		else:
			print('{system} is not supported by FSMTP-V2')


def install_repository(url, dir):
	print(f'Installing repository \'{url}\' into \'{dir}\'')

	os.system('mkdir -p ./dependencies')
	os.chdir('./dependencies')
	os.system(f'git clone --recursive {url} ./{dir}')
	os.chdir(f'./{dir}')
	os.system('mkdir -p ./build')
	os.chdir('./build')
	os.system(f'cmake ../ && make -j {THREADS} && make install')
	os.chdir('../../../')

def install_dependencies(template):
	commands = list()

	if (template == InstallationTemplate.ArchLinux):
		commands.append('pacman -Syyy');
		commands.append('pacman -S meson ninja python python-pip gcc make cmake openssl pkg-config')
	elif (template == InstallationTemplate.DebianLinux):
		commands.append('apt-get update')
		commands.append('apt-get upgrade')
		commands.append('apt-get install ninja-build meson python3 python3-pip gcc make cmake openssl pkg-config')
	elif (template == InstallationTemplate.AlpineLinux):
		commands.append('apk add build-base openssl meson make cmake ninja git libuv')

	commands.append('ldconfig')
	commands.append('pip3 install pyOpenSSL cassandra-driver || pip install pyOpenSSL cassandra-driver')

	print(f'[DEPENDENCIES] Starting execution of {len(commands)} commands')
	for i, command in enumerate(commands):
		print(f'{i} -> Executing command \'{command}\'')
		os.system(command)


def install():
	commands = list()

	commands.append('mkdir -p env && mkdir -p env/keys')
	commands.append('python ./initdb.py')
	commands.append('python ./gencert.py')

	commands.append('mkdir -p env && touch env/meson.build')
	commands.append('meson build && cd ./build && ninja')

	print(f'Starting execution of {len(commands)} commands')
	for i, command in enumerate(commands):
		print(f'{i} -> Executing command \'{command}\'')
		os.system(command)

"""
	Starts the installation
"""

template = InstallationTemplate.detect()
print(f'Detected template: {template}')
if (template == InstallationTemplate.UnknownLinux):
	sys.exit(0)

install_dependencies(template)
install_repository('https://github.com/onqtam/doctest', 'doctest')
install_repository('https://github.com/nickbruun/hayai', 'hayai')
install_repository('https://github.com/nlohmann/json', 'json')
install_repository('https://github.com/open-source-parsers/jsoncpp', 'jsoncpp')
install_repository('https://github.com/pantor/inja', 'inja')
install_repository('https://github.com/redis/hiredis', 'hiredis')
install_repository('https://github.com/datastax/cpp-driver', 'cpp-driver')
install_repository('https://github.com/catchorg/Catch2', 'Catch2')
install()

"""
	Generates and installs the service
"""

service = f"""[Unit]
Description=FSMTP-V2 Email server

[Service]
Type=simple
ExecStart={os.getcwd()}/build/fsmtp
WorkingDirectory={os.getcwd()}/build
Restart=on-failure

[Install]
WantedBy=multi-user.target"""

with open('/lib/systemd/system/fsmtp.service', 'w+') as f:
	f.write(service)

os.system('systemctl daemon-reload')
os.system('systemctl enable fsmtp && systemctl start fsmtp')