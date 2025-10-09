// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// This file is part of eProsima OpenDDS Visualizer Plugin.
//
// eProsima OpenDDS Visualizer Plugin is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// eProsima OpenDDS Visualizer Plugin is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with eProsima OpenDDS Visualizer Plugin. If not, see <https://www.gnu.org/licenses/>.

/**
 * @file OpenDdsDataStreamer.hpp
 */

#ifndef _EPROSIMA_PLOTJUGGLEROPENDDS_PLUGIN_PLUGINS_DATASTREAMERPLUGIN_DATASTREAMER_DATASTREAMER_HPP_
#define _EPROSIMA_PLOTJUGGLEROPENDDS_PLUGIN_PLUGINS_DATASTREAMERPLUGIN_DATASTREAMER_DATASTREAMER_HPP_

#include <QtPlugin>

#include <PlotJuggler/datastreamer_base.h>

#include "ui/topic_selection_dialog/dialogselecttopics.h"
#include "ui/topic_selection_dialog/Configuration.hpp"
#include "ui/topic_selection_dialog/UiListener.hpp"
#include "opendds/OpenDdsListener.hpp"
#include "opendds/Handler.hpp"
#include "utils/types.hpp"

namespace eprosima {
namespace plotjuggler {
namespace datastreamer {

/**
 * @brief OpenDDS Data Streamer for PlotJuggler
 */
class OpenDdsDataStreamer :
    public PJ::DataStreamer,
    public opendds::OpenDdsListener,
    public ui::UiListener
{
    //! Macros for Qt Plugin and Object
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "facontidavide.PlotJuggler3.OpenDDSVisualizerPlugin")
    Q_INTERFACES(PJ::DataStreamer)

public:

    /**
     * @brief Construct a new OpenDDS Data Streamer object
     *
     * @note This object is constructed at the beginning of the execution of the program,
     * even before the plugin is selected and started.
     */
    OpenDdsDataStreamer();

    /**
     * @brief Construct a new OpenDDS Data Streamer object
     *
     * @note This occurs once at the end of the process when the UI closes
     */
    ~OpenDdsDataStreamer();


    ////////////////////////////////////////////////////
    // DATASTREAMER PLUGIN METHODS
    ////////////////////////////////////////////////////

    bool start(
            QStringList*) override;

    void shutdown() override;

    bool isRunning() const override;

    const char* name() const override;

    bool xmlSaveState(
            QDomDocument& doc,
            QDomElement& plugin_elem) const override;

    bool xmlLoadState(
            const QDomElement& parent_element) override;

    ////////////////////////////////////////////////////
    // OPENDDS LISTENER METHODS
    ////////////////////////////////////////////////////
    virtual void on_data_available() override;

    virtual void on_double_data_read(
            const std::vector<types::NumericDatum>& data_per_topic_value,
            double timestamp) override;

    virtual void on_string_data_read(
            const std::vector<types::TextDatum>& data_per_topic_value,
            double timestamp) override;

    virtual void on_topic_discovery(
            const std::string& topic_name,
            const std::string& type_name) override;


    ////////////////////////////////////////////////////
    // UI LISTENER METHODS
    ////////////////////////////////////////////////////

    virtual void on_xml_datatype_file_added(
            const std::string& file_path) override;

    virtual void on_domain_connection(
            unsigned int domain_id) override;


    ////////////////////////////////////////////////////
    // STATIC METHODS AND PUBLIC MEMBERS
    ////////////////////////////////////////////////////

    constexpr static const char* PLUGIN_NAME_ = "OpenDDS Visualizer Plugin";

protected:

    ////////////////////////////////////////////////////
    // AUXILIAR METHODS
    ////////////////////////////////////////////////////

    void connect_to_domain_(
            unsigned int domain_id);

    void create_series_();

    ////////////////////////////////////////////////////
    // INTERNAL VALUES
    ////////////////////////////////////////////////////

    ui::Configuration configuration_;

    opendds::Handler opendds_handler_;

    ui::DialogSelectTopics select_topics_dialog_;

    bool running_;

    constexpr static const char* CONFIGURATION_SETTINGS_PREFIX_ = "OpenDDSVisualizerPlugin";
};

} /* namespace datastreamer */
} /* namespace plotjuggler */
} /* namespace eprosima */

#endif // _EPROSIMA_PLOTJUGGLEROPENDDS_PLUGIN_PLUGINS_DATASTREAMERPLUGIN_DATASTREAMER_DATASTREAMER_HPP_
