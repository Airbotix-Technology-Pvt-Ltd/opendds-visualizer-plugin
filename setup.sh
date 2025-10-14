#!/bin/bash
#
# PlotJuggler Setup Script (Simplified)
#
# Usage:
#   source ./setup.sh
#   # or
#   . ./setup.sh
#
# After sourcing:
#   To make it permanent, run the single command below that matches your shell:
#     For Bash:
#       echo "export PLOTJUGGLER_DIR=\"$(pwd)/docker\"" >> ~/.bashrc
#       echo "source \"$(pwd)/setup.sh\"" >> ~/.bashrc
#     For Zsh:
#       echo "export PLOTJUGGLER_DIR=\"$(pwd)/docker\"" >> ~/.zshrc
#       echo "source \"$(pwd)/setup.sh\"" >> ~/.zshrc
#

export PLOTJUGGLER_DIR="${PLOTJUGGLER_DIR:-$(pwd)/docker}"
PROJECT_NAME="plotjuggler_project"
CONTAINER="plotjuggler"

plotjuggler() {
  case "$1" in
    start)
      if [ ! -d "$PLOTJUGGLER_DIR" ]; then
        echo "$PLOTJUGGLER_DIR not found. Make sure Docker files exist."
        return 1
      fi
      echo "Starting PlotJuggler container..."
      xhost +local:docker
      docker compose --project-directory "$PLOTJUGGLER_DIR" -p "$PROJECT_NAME" up
      ;;
    build)
      if [ ! -f "$PLOTJUGGLER_DIR/docker-compose.yml" ]; then
        echo "docker-compose.yml not found. Please ensure it's available in $PLOTJUGGLER_DIR"
        return 1
      fi
      echo "Building PlotJuggler container..."
      docker compose --project-directory "$PLOTJUGGLER_DIR" -p "$PROJECT_NAME" build
      ;;
    rebuild)
      if [ ! -d "$PLOTJUGGLER_DIR" ]; then
        echo "$PLOTJUGGLER_DIR not found."
        return 1
      fi
      echo "Rebuilding PlotJuggler container..."
      docker compose --project-directory "$PLOTJUGGLER_DIR" -p "$PROJECT_NAME" build --no-cache
      ;;
    restart)
      docker compose --project-directory "$PLOTJUGGLER_DIR" -p "$PROJECT_NAME" restart
      ;;
    stop)
      docker compose --project-directory "$PLOTJUGGLER_DIR" -p "$PROJECT_NAME" down
      ;;
    kill)
      docker kill "$CONTAINER" 2>/dev/null || echo "Container not running"
      ;;
    -h|--help|help|*)
      cat << 'HELP_EOF'
Usage: plotjuggler [COMMAND]

Commands:
  start     - Start PlotJuggler container
  build     - Build PlotJuggler image
  rebuild   - Rebuild image (no cache)
  restart   - Restart container
  stop      - Stop and remove container
  kill      - Force kill container
  help      - Show this help

HELP_EOF
      ;;
  esac
}

# Export function when sourced and print tailored instructions
if [[ "${BASH_SOURCE[0]}" != "${0}" ]]; then
  export -f plotjuggler
  echo "plotjuggler function available in this shell."
  plotjuggler -h
  if [ -n "$ZSH_VERSION" ]; then
    echo "To make it permanent, run these commands for Zsh:"
    echo "  echo \"export PLOTJUGGLER_DIR=\\\"$(pwd)/docker\\\"\" >> ~/.zshrc"
    echo "  echo \"source \\\"$(pwd)/setup.sh\\\"\" >> ~/.zshrc"
  elif [ -n "$BASH_VERSION" ]; then
    echo "To make it permanent, run these commands for Bash:"
    echo "  echo \"export PLOTJUGGLER_DIR=\\\"$(pwd)/docker\\\"\" >> ~/.bashrc"
    echo "  echo \"source \\\"$(pwd)/setup.sh\\\"\" >> ~/.bashrc"
  else
    echo "Could not detect your shell. Please add the following lines to your shell config file:"
    echo "  export PLOTJUGGLER_DIR=\"$(pwd)/docker\""
    echo "  source \"$(pwd)/setup.sh\""
  fi
fi
