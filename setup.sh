#!/bin/bash
#
# PlotJuggler Setup Script
#
# Usage:
#   source ./setup.sh
#   # or
#   . ./setup.sh
#
# This will:
#   1. Add plotjuggler function to your .bashrc/.zshrc
#   2. Download required Docker files
#   3. Make the function available immediately
#

# PlotJuggler Docker Function
plotjuggler() {
  local CMD_DIR=/opt/plotjuggler
  local PROJECT_NAME=plotjuggler_project
  local CONTAINER=plotjuggler

  case "$1" in
    "")
      if ! docker ps --format '{{.Names}}' | grep -q "^${CONTAINER}$"; then
        echo "Container '$CONTAINER' is not running. Start it first with 'plotjuggler start'"
        return 1
      fi
      echo "Opening PlotJuggler..."
      docker exec -it "$CONTAINER" bash -c \
        "source /DDS-Visualizer-Plugin/install/setup.bash && \
         plotjuggler -n --plugin_folders /DDS-Visualizer-Plugin/install/opendds_visualizer_plugin/bin/"
      ;;

    pull)
      echo "Pulling PlotJuggler..."
      sudo rm -rf "$CMD_DIR"

      echo "Setting up PlotJuggler..."
      sudo mkdir -p "$CMD_DIR"
        
      # Download docker-compose.yml
      sudo wget -O "$CMD_DIR/docker-compose.yml" \
        https://raw.githubusercontent.com/Airbotix-Technology-Pvt-Ltd/opendds-visualizer-plugin/opendds/docker/docker-compose.yml

      # Download Dockerfile
      sudo wget -O "$CMD_DIR/Dockerfile" \
        https://raw.githubusercontent.com/Airbotix-Technology-Pvt-Ltd/opendds-visualizer-plugin/opendds/docker/Dockerfile

      # Download plotjuggler.sh
      sudo wget -O "$CMD_DIR/setup.sh" \
        https://raw.githubusercontent.com/Airbotix-Technology-Pvt-Ltd/opendds-visualizer-plugin/opendds/setup.sh

      echo "Setup complete at $CMD_DIR"
      ;;

    build)
      if [ ! -f "$CMD_DIR/docker-compose.yml" ]; then
        echo "docker-compose.yml not found in $CMD_DIR"
        echo "Run 'plotjuggler setup' first"
        return 1
      fi
      
      echo "Building PlotJuggler container..."
      docker compose --project-directory "$CMD_DIR" -p "$PROJECT_NAME" build
      ;;

    rebuild)
      if [ ! -d "$CMD_DIR" ]; then
        echo "Directory $CMD_DIR not found."
        return 1
      fi
      
      echo "Rebuilding PlotJuggler container..."
      docker compose --project-directory "$CMD_DIR" -p "$PROJECT_NAME" build --no-cache
      ;;

    start)
      if [ ! -d "$CMD_DIR" ]; then
        echo "Directory $CMD_DIR not found. First build the container."
        return 1
      fi
      
      echo "Starting PlotJuggler container..."
      xhost +local:docker
      docker compose --project-directory "$CMD_DIR" -p "$PROJECT_NAME" up -d
      ;;

    restart)
      if [ ! -d "$CMD_DIR" ]; then
        echo "Directory $CMD_DIR not found."
        return 1
      fi
      
      echo "Restarting PlotJuggler container..."
      docker compose --project-directory "$CMD_DIR" -p "$PROJECT_NAME" restart
      ;;

    stop)
      echo "Stopping PlotJuggler container..."
      docker compose --project-directory "$CMD_DIR" -p "$PROJECT_NAME" down
      ;;

    kill)
      echo "Killing PlotJuggler container..."
      docker kill "$CONTAINER" 2>/dev/null || echo "Container not running"
      ;;

    -h|--help|help)
      cat << 'HELP_EOF'
Usage: plotjuggler [COMMAND]

Commands:
  (none)    - Open PlotJuggler in running container
  pull     - Pull latest PlotJuggler
  build     - Build the Docker image
  rebuild   - Rebuild the Docker image (no cache)
  start     - Start PlotJuggler container
  restart   - Restart the container
  stop      - Stop and remove the container
  kill      - Force kill the container
  -h, help  - Show this help message

Examples:
  plotjuggler                    # open plotjuggler
  plotjuggler pull               # Pull latest PlotJuggler
  plotjuggler build              # Build the image
  plotjuggler rebuild            # Rebuild the image
  plotjuggler restart            # Restart the container
  plotjuggler start              # Start PlotJuggler container
  plotjuggler stop               # Stop container
  plotjuggler -h                 # Show help
  plotjuggler kill               # Kill the container
HELP_EOF
      ;;

    *)
      echo "Unknown command: $1"
      echo "Run 'plotjuggler -h' for usage information"
      return 1
      ;;
  esac
}

# Add to .bashrc or .zshrc
if ! grep -q "plotjuggler()" ~/.bashrc; then
  echo "Adding 'plotjuggler' function to ~/.bashrc"
  plotjuggler pull
  echo "source "/opt/plotjuggler/setup.sh"" >> ~/.bashrc
fi