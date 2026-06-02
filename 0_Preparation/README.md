<a name="top"></a>
# Preparing for the SDSC Summer Institute 2026

Please complete the following before the institute begins. Being prepared will let you focus on the material rather than setup during the event.

## Preparation Checklist

- [ ] **Set up your accounts:**
    - [ ] [Create a GitHub account](#github).
    - [ ] Look for an email about your Expanse training account.
- [ ] **Laptop setup:**
    - [ ] [Install an SSH client](#computer-requirements) on your laptop.
    - [ ] [Install Slack](#slack) and join the workspace.
    - [ ] [Install Zoom](#zoom).
- [ ] **Review basic HPC skills:**
    - [ ] [Connecting Securely to SDSC HPC Systems](#basic-hpc-skills)
    - [ ] [Basic Linux Skills for Expanse](#basic-hpc-skills)
    - [ ] [Interactive Computing on Expanse](#basic-hpc-skills)
    - [ ] [Using GitHub on Expanse](#basic-hpc-skills)
- [ ] **Attend Preparation Day:**
    - [ ] Join the virtual Prep Day on Thursday, July 23, 2026, from 9am–11am (Pacific Time).

---

## Computer Requirements

The Summer Institute is a hands-on program. You need a laptop with **an SSH client and a web browser**. Administrative privileges are **not** required.

### SSH Client

All hands-on exercises run on the Expanse supercomputer — you only need to SSH into it from your laptop.

**Recommended: OpenSSH** (built-in on macOS and Linux, available on Windows 10+)

- **macOS / Linux:** OpenSSH is pre-installed. Open Terminal and run `ssh` to verify.
- **Windows:** Enable the [OpenSSH Client in PowerShell](https://learn.microsoft.com/en-us/windows-server/administration/openssh/openssh_install_firstuse?tabs=gui). Alternatively, install the [Windows Subsystem for Linux (WSL)](https://learn.microsoft.com/en-us/windows/wsl/install) for a full Linux environment.

**Alternative for Windows:** [PuTTY](https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html)

### Other

- WiFi will be available throughout the SDSC building.
- Remember your charger — extension cords will be provided.

[Back to Top](#top)
<hr>

## Expanse User Guide

The [Expanse User Guide](https://www.sdsc.edu/support/user_guides/expanse.html) is your comprehensive reference. Key points:

- **System Architecture:** Standard compute nodes, GPU nodes (NVIDIA V100s), and large-memory nodes.
- **File Systems:**
    - `$HOME` (`/home`): 100GB for source code and configs (backed up). **Do not run jobs from here.**
    - `/expanse/lustre/scratch`: High-performance temporary storage. **Not backed up; files purged after 90 days.**
    - `/expanse/lustre/projects`: Project-specific storage.
    - `/scratch/$USER/job_$SLURM_JOB_ID`: Fast local storage on compute nodes, available only during job execution.
- **Connecting:** SSH with two-factor authentication (2FA).
- **Software Environment:** Manage software with `module`. Use `module spider <package_name>` to find packages.
- **Running Jobs:** Submit with `sbatch`, use `srun` for interactive sessions. Example scripts in `/cm/shared/examples/sdsc/`.

[Back to Top](#top)
<hr>

## HPC Systems Accounts

A training account on SDSC's Expanse system will be created for you. You will receive an email from consult@sdsc.edu with the subject "HPC & Data Science 2026 Summer Institute: Account Set-Up" containing your username, password, and instructions.

Please use the training account provided, even if you have your own Expanse account — this avoids issues from custom configurations (environment variables, shell choice, etc.) that may break the hands-on examples.

Access your training account via:
- SSH to `login.expanse.sdsc.edu`
- The Expanse Training Portal: https://portal.expanse.sdsc.edu/training

[Back to Top](#top)
<hr>

## Preparation Activities

### Basic HPC Skills

- [Connecting Securely to SDSC HPC Systems](https://github.com/sdsc-hpc-training-org/hpc-security): SSH, passwordless login with SSH keys, and two-factor authentication.
    - WATCH — [Indispensable Security: Tips to Use SDSC's HPC Resources Securely](https://education.sdsc.edu/training/interactive/202007_security_tips/index.php)
- [Basic Linux Skills for Expanse](https://github.com/sdsc-hpc-training-org/basic_skills/tree/master/basic_linux_skills_expanse): Fundamental Linux commands for navigating the file system, managing files, and understanding permissions.
- [Interactive Computing on Expanse](https://github.com/sdsc-hpc-training-org/basic_skills/tree/master/interactive_computing): Requesting and using interactive sessions on CPU and GPU nodes.
- [Using GitHub on Expanse](https://github.com/sdsc-hpc-training-org/basic_skills/tree/master/using_github): Setting up and using Git and GitHub on SDSC systems.

### Launching Jupyter Notebooks

- Via the Expanse Portal: https://portal.expanse.sdsc.edu

[Back to Top](#top)
<hr>

## GitHub, Slack, and Zoom

### GitHub

Training material is on the [SDSC Summer Institute 2026 GitHub repo](https://github.com/sdsc/sdsc-summer-institute-2026). One of the Wednesday sessions requires a GitHub account. If you don't have one, create a free account [here](https://docs.github.com/en/github/getting-started-with-github/signing-up-for-github/signing-up-for-a-new-github-account).

For GitHub usage on SDSC systems, see [here](https://github.com/sdsc-hpc-training-org/basic_skills/tree/master/using_github).

### Slack

Slack is our main platform for announcements and Q&A during the institute. Download for [Windows](https://slack.com/downloads/windows), [macOS](https://slack.com/downloads/mac), or [Linux](https://slack.com/downloads/linux). Enable notifications to receive alerts.

See the "Preparation Information | HPC & Data Science Summer Institute 2026" email for the link to join the Slack workspace.

### Zoom

Zoom is used for the virtual Preparation Day on **Thursday, July 23, 2026, from 9am–11am (Pacific Time)**. Install the latest [Zoom](https://zoom.us/download) client and [test your setup](https://zoom.us/test).

Connection details will be sent as a calendar invite.

Note: The main institute (August 3–7) is in-person only and will not be available remotely.

[Back to Top](#top)
