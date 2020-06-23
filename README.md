# CS3099 A-3 Group Project

## Overview
Micro:Bits connect to each other to exchange social information using the onboard radio, and can connect to the internet to retrieve social media notifications.
A Micro:Bit being used by a person is referred to as a *active MB*.
A Micro:Bit connected to a PC (referred to as a *server*) is referred to as a *gateway MB*.

There are two distinct software endeavors enclosed within this respository: that of the server, and the code that runs on the Micro:Bit for both active and gateway purposes (a single code base is used for the Micro:Bit to reduce complexity).


## Contributing
All features must be developed in separate branches.  When ready to be merged, a merge request must be created, and the commits must pass continuous integration tests.  The commits must then be verified by another group member before being merged.

When creating a merge request, ensure the following:
- **Approvals required** is set to *1*
- **Squash commits when merge request is accepted** is *checked*


## Microbit code base
- Ensure current directory is correct (/microbit)
- Build with `yt build`
- Copy the generated `.hex` file to the Microbit

## Server code base (Twitter relay)
- Only supported on Linux (serial relay)
- Ensure current directory is correct (/server)
- Run connector application via `pipenv run python mbit_connector.py`

## Web app
- Navigate to server/gapp
- If virtual environment folder doesn't exist, create one using `python –m venv virtual`
- Activate environment using `source virtual/bin/activate`
- Install flask and requests if necessary
- Run using `flask run`


## Server code base (Django)
- Python 3, dependencies managed using [pipenv](https://github.com/pypa/pipenv)
- Interact using Django
- DB backend is sqlite3, not stored in git
- Ensure the below commands are run from the `/server` directory (cd `server` from repo root)

### Setup
- Install latest dependencies: `pipenv install`

### Migrating DB
- Run migrations: `pipenv run python manage.py migrate`

### Creating a superuser
- Create superuser: `pipenv run python manage.py createsuperuser`

### Running
- Run server: `pipenv run python manage.py runserver`
