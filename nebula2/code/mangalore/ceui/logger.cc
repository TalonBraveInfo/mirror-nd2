//------------------------------------------------------------------------------
//  ceui/logger.cc
//  (c) 2006 Nebula2 Community
//------------------------------------------------------------------------------
#include "ceui/logger.h"
#include "kernel/nkernelserver.h"

namespace CEUI
{

//------------------------------------------------------------------------------
/**
*/
Logger::Logger() {
}

//------------------------------------------------------------------------------
/**
*/
Logger::~Logger() {
}

//------------------------------------------------------------------------------
/**
*/
void Logger::logEvent(const CEGUI::String& message, CEGUI::LoggingLevel level) {
    if (level <= getLoggingLevel()) {
        switch (level) {
        case CEGUI::Errors:
            nKernelServer::Instance()->Error("%s\n", message.c_str());
            break;
        case CEGUI::Standard:
            nKernelServer::Instance()->Print("%s\n", message.c_str());
            break;
        case CEGUI::Informative:
            nKernelServer::Instance()->Print("%s\n", message.c_str());
            break;
        case CEGUI::Insane:
            nKernelServer::Instance()->Print("%s\n", message.c_str());
            break;
        default:
            n_error("Unknown CEGUI logging level\n");
        }
    }
}

//------------------------------------------------------------------------------
/**
*/
void Logger::setLogFilename(const CEGUI::String& filename, bool append) {
}

} // namespace CEUI
