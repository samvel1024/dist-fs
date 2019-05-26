push:
	rsync -uav -e "ssh -p 22" --exclude=".idea" --exclude=".git" --exclude="cmake-build-debug" ./ root@${TARGET_IP}:/root/sik
