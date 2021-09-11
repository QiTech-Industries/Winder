#!/bin/bash

cp -r /usr/src/cache/node_modules/. /usr/src/app/node_modules/
exec npm run serve