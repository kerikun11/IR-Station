import subprocess

revision = (
    subprocess.check_output(["git", "describe", "--tags", "--always"]
                            ).strip() .decode("utf-8")
)
print("'-D GIT_DESCRIBE=\"%s\"'" % revision)
