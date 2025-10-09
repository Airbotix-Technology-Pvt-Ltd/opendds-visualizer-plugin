# OpenDDS Visualizer Plugin

<br>

<div class="badges" align="center">
    <a href="https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html"><img alt="License" src="https://img.shields.io/github/license/Airbotix-Technology-Pvt-Ltd/opendds-visualizer-plugin.svg"/></a>
    <a href="https://github.com/Airbotix-Technology-Pvt-Ltd/opendds-visualizer-plugin/releases"><img alt="Releases" src="https://img.shields.io/github/v/release/Airbotix-Technology-Pvt-Ltd/opendds-visualizer-plugin?sort=semver"/></a>
    <a href="https://github.com/Airbotix-Technology-Pvt-Ltd/opendds-visualizer-plugin/issues"><img alt="Issues" src="https://img.shields.io/github/issues/Airbotix-Technology-Pvt-Ltd/opendds-visualizer-plugin.svg"/></a>
    <a href="https://github.com/Airbotix-Technology-Pvt-Ltd/opendds-visualizer-plugin/network/members"><img alt="Forks" src="https://img.shields.io/github/forks/Airbotix-Technology-Pvt-Ltd/opendds-visualizer-plugin.svg"/></a>
    <a href="https://github.com/Airbotix-Technology-Pvt-Ltd/opendds-visualizer-plugin/stargazers"><img alt="Stars" src="https://img.shields.io/github/stars/Airbotix-Technology-Pvt-Ltd/opendds-visualizer-plugin.svg"/></a>
</div>

<br><br>

*OpenDDS Visualizer Plugin* is a plugin for the *PlotJuggler* application.
PlotJuggler is a graphical desktop application providing visualization features
of data series, time series, X-Y plots.
It also adds data management features, such as
data import and export, custom and built-in data manipulation functions,
data series merges, etc.
Also, this software supports many different layouts, with dynamic, rich and user-friendly customization.

*OpenDDS Visualizer Plugin* allows users to visualize topic-related data from a DDS network.
The user can select several topics from the discovered topics running in the DDS network.
These topics will be divided by values using data type introspection,
and each value could be visualized and managed separately, allowing the user to quickly visualize
in different ways the detailed data content that is being
exchanged in the network.

*OpenDDS Visualizer Plugin* supports the following features:

1. **Data type introspection**: by using *OpenDDS Dynamic Types*,
   this plugin allows to discover the data type of the topic,
    and to visualize the data content using the corresponding data type.
1. **DDS Configurations**: Different configurations can be set to choose the *Domain Id* and to
   select specific *DDS Topics* to be visualized.
1. **All PlotJuggler features**: This plugin composes nicely with all the PlotJuggler features,
   so that users can create rich graphs from sophisticated data manipulations.

## About

This plugin is a port of the original [Fast DDS Visualizer Plugin](https://github.com/eProsima/fastdds-visualizer-plugin) 
by eProsima, adapted to work with OpenDDS instead of Fast DDS.

## Links

* [OpenDDS Documentation](https://opendds.org/)
* [PlotJuggler](https://github.com/facontidavide/PlotJuggler)
* [Original Fast DDS Plugin](https://github.com/eProsima/fastdds-visualizer-plugin)
