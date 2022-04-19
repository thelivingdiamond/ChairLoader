# Chairloader
Chairloader is a modding framework built for Prey (2017) released by Arkane. It is primarily built to enable real time code execution, injection, and hooking for the game. This includes things like spawning entities and items, manipulating entities, accessing internal game functions like free-cam, and much more. This repository isn't built to be a place to get mods, it's intended to hold the tools needed to make whatever mods your heart desires. 

**Be warned, this is extremely experimental code at the moment. I don't have much in terms of documentation or examples yet. This is the cutting edge of runtime modding for this game and as such, things will change sporadically and quickly. Many many things you can do will crash the game. This is very much a WIP project**

## What can Chairloader do right now?
The current list of capabilities includes:
- NPC Spawning (fully functional)
- Entity Spawning (semi functional)
- NPC and Entity Manipulation (fully functional)
- Ability to execute internal console commands
- Ability to spawn custom NPCs and entities added to the XML files
- A mapped library of all pre-existing entity archetypes and their ID's

An important note is that this is primarily designed for things that appear or happen spontaneously. If you need something to always exist in a level then XML file manipulation is still your best bet

## Todo List
- Working Cazavor (Cut Disc-Rifle weapon)
- Generally more extensive mapping of functions and objects to enable more functionality
- Documentation and Wiki (long term)

And lastly I'm still a full time university student so please be lenient about consistent updates here. Working on this modding framework is very time intensive
