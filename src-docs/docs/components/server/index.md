![META](authors:Nicolas "Duduf" Dufresne;license:GNU-FDL;copyright:2021;updated:2021/07/03)

# The Ramses Server

The server stores and serves all shared data to the *Ramses Client* between all workstations in the studio. The *Ramses Clients* installed on these workstations connect to it to retrieve tha data they need.

It is just a small software meant to be installable on small hardware and with very low requirements. You can install it on any computer in the studio (even just a simple [*Raspberry*](https://www.raspberrypi.org/)) and make it available on your *Local Area Network*, or make it available from the internet. You can also easily install it on a shared or private hosting, even a low-cost one.

!!! tip
    Although the currently available *Beta Version* of the *Ramses Client* cannot be used as a stand-alone, offline application yet and needs a connection to a *Ramses Server*, you can install a local server on any computer to test and use *Ramses* without needing a full scale online server. We're explaining in this documentation how to install such a standard webserver on your computer, read below.

## Requirements

The *Ramses Server* must be installed on a computer with:

- An **http web server** like *Apache*
- ***PHP 7***
- An acces to a ***mySQL*** server and a dedicated database.

The recommended configuration is a standard *AMP* stack (*Apache* - *mySQL* - *PHP*), with at least 2GB of RAM if the *SQL* server is on the same computer than the *Apache* server.

- The needed space for the *Ramses Server* files (*PHP*) is only *`300 KB`*!
- *`50 MB`* should be more than enough for the vast majority of users for the *mySQL* database.

!!! note
    As *Ramses* is still in development, the only compatible SQL database is *mySQL*. We may add support for other databases in the future, include *SQLite* so the *Ramses Server* could be use without an actual *SQL* database.

We provide a detailed, step-by-step guide to help you install your own web server satisfying these requirements on *Windows*, *Linux* or *Mac OS* if you need to, before installing the *Ramses Server*: [install your own webserver](web-server.md).

As soon as you have a working web server, you can [install the *Ramses server*](install.md).

## Technical description

The *Ramses Server* is a pretty standard *REST API* written in *PHP* which just serves as an interface to an *SQL* database. It receives requests from the *Ramses Client* and responds with the needed data.

For developers who may want to extend or modify *Ramses*, or develop their own client, [the complete reference of this *API* is available here](../../dev/server-reference/index.md).