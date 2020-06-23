#!/usr/bin/env bash
case $1 in
    runserver)
        cd server/flaskapp
        pipenv run python -m flask run
    ;;
    flash)
        cd microbit
        make flash
    ;;
    flash-both)
        cd microbit
        make flashb
    ;;
esac