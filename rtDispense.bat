@echo off
docker build . -f Dispense.amd.Dockerfile -t good
if errorlevel 1 exit /b %errorlevel%
set TEST_IMAGE=good
docker compose -f .github/compose/tests-ci.yml up tests
