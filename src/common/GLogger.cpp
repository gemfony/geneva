/**
 * @file GLogger.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

#include "common/GLogger.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor. No local data, hence empty
 */
GBaseLogTarget::GBaseLogTarget(void)
{ /* nothing */ }

/***********************************************************************************/
/**
 * The standard destructor. No local data, hence empty
 */
GBaseLogTarget::~GBaseLogTarget()
{ /* nothing */ }

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor. No local data, hence empty
 */
GConsoleLogger::GConsoleLogger(void)
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor. No local data, hence empty
 */
GConsoleLogger::~GConsoleLogger()
{ /* nothing */ }

/******************************************************************************/
/**
 * Logs a message to the console.
 *
 * @param msg The log message
 */
void GConsoleLogger::log(const std::string& msg) const {
   std::clog << msg;
}

/******************************************************************************/
/**
 * Logs a message to the console, adding information about the source
 *
 * @param msg The log message
 */
void GConsoleLogger::logWithSource(
      const std::string& msg
      , const std::string& extension
) const {
   this->log(std::string("Message from source \"") + extension + "\":\n" + msg);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
GFileLogger::GFileLogger(void)
   : fname_("Gemfony.log")
   , first_(true)
{ /* nothing */ }

/*******************************************************************************/
/**
 * This constructor accepts a boost path to a file name as argument
 */
GFileLogger::GFileLogger(const boost::filesystem::path& p)
   : fname_(p.string())
   , first_(true)
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor.
 */
GFileLogger::~GFileLogger()
{ /* nothing */ }

/******************************************************************************/
/**
 * This function logs a message to a file, whose name it takes from the private
 * variable fname_. The file is reopened in append mode for every log message.
 */
void GFileLogger::log(const std::string& msg) const {
   std::ofstream ofstr(fname_.c_str(), std::ios_base::app);
   if(ofstr){
      ofstr << msg;
      ofstr.close();
   }
   else{
      std::cerr
      << "In GFileLogger::log() : Error" << std::endl
      << "std::ofstring is in a bad state" << std::endl;
      exit(1);
   }
}

/******************************************************************************/
/**
 * This function logs a message to a file, whose name it takes from the private
 * variable fname_. The file is reopened in append mode for every log message.
 * In addition to the standard log() function, this function appends the logging
 * source to the file name
 */
void GFileLogger::logWithSource(
      const std::string& msg
      , const std::string& extension
) const {
   std::ofstream ofstr((fname_ + "_" + extension). c_str(), std::ios_base::app);
   if(ofstr){
      if(first_) {
         ofstr
         << "Logging data from source " + extension << std::endl
         << msg;
         ofstr.close();
         first_ = false;
      } else {
         ofstr << msg;
         ofstr.close();
      }
   }
   else{
       std::cerr
       << "In GFileLogger::logWithSource() : Error" << std::endl
       << "std::ofstring is in a bad state" << std::endl;
       exit(1);
   }
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A constructor that takes both accompanying information and the
 * desired log type as an argument and stores the information for
 * later perusal.
 */
GManipulator::GManipulator(
      const std::string& accompInfo
      , const logType& lt
)
   : accompInfo_(accompInfo)
   , logType_(lt)
{ /* nothing */ }

/******************************************************************************/
/**
 * A constructor that stores the logging type only
 */
GManipulator::GManipulator(const logType& lt)
   : accompInfo_()
   , logType_(lt)
{ /* nothing */ }

/******************************************************************************/
/**
 * Retrieves the stored logging type
 */
logType GManipulator::getLogType() const {
   return logType_;
}

/******************************************************************************/
/**
 * Retrieves stored accompanying information (if any)
 */
std::string GManipulator::getAccompInfo() const {
   return accompInfo_;
}

/******************************************************************************/
/**
 * Checks whether any accompanying information is available
 */
bool GManipulator::hasAccompInfo() const {
   return !accompInfo_.empty();
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor.
 */
GLogStreamer::GLogStreamer(void)
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor.
 */
GLogStreamer::GLogStreamer(const GLogStreamer& cp)
   : oss_(cp.oss_.str())
   , extension_(cp.extension_)
   , logFile_(cp.logFile_)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with an optional string source extension for log files
 * or as additional information in std-output logs
 */
GLogStreamer::GLogStreamer(const std::string& extension)
   : oss_()
   , extension_(extension)
   , logFile_()
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with the name and path of a file used for
 * one-time logging
 */
GLogStreamer::GLogStreamer(boost::filesystem::path logFile)
   : oss_()
   , extension_()
   , logFile_(logFile)
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard destructor.
 */
GLogStreamer::~GLogStreamer()
{ /* nothing */ }

/******************************************************************************/
/**
 * Needed for ostringstream
 */
GLogStreamer& GLogStreamer::operator<< (std::ostream& ( *val )(std::ostream&)){
   oss_ << val;
   return *this;
}

/******************************************************************************/
/**
 * Needed for ostringstream
 */
GLogStreamer& GLogStreamer::operator<< (std::ios& ( *val )(std::ios&)){
   oss_ << val;
   return *this;
}

/******************************************************************************/
/**
 *  Needed for ostringstream
 */
GLogStreamer& GLogStreamer::operator<< (std::ios_base& ( *val )(std::ios_base&)){
   oss_ << val;
   return *this;
}

/******************************************************************************/
/**
 * Interface to the actual logging mechanism. Note that this function does
 * not return a reference to self, as it is meant to be called as the last
 * element of a streaming-chain.
 *
 * @param gm A GManipulator object, usually emitted by the logLevel() function.
 */
void GLogStreamer::operator<<(const GManipulator& gm){
   switch(gm.getLogType()) {
      //------------------------------------------------------------------------
      case Gem::Common::EXCEPTION:
      {
         // Assemble the output string
         std::ostringstream error;
         error
            << std::endl
            << "================================================" << std::endl
            << "ERROR (recorded on " << boost::posix_time::second_clock::local_time() << ")" << std::endl
            << gm.getAccompInfo() << std::endl
            << std::endl
            << oss_.str()
            << std::endl
            << "If you suspect that this error is due to Geneva," << std::endl
            << "then please consider filing a bug via" << std::endl
            << "http://www.gemfony.com (link \"Bug Reports\") or" << std::endl
            << "through http://www.launchpad.net/geneva" << std::endl
            << std::endl
            << "We appreciate your help!" << std::endl
            << "The Geneva team" << std::endl
            << "================================================" << std::endl;

         // Do all necessary logging. We use a central exception file for this purpose.
         // The logger's routines will in addition try to print on the console. Note
         // that we ignore any "source" information, as this is already contained
         // in the logged message.
         GFileLogger gfl("GENEVA-EXCEPTION.log");
         gfl.log(error.str());

         // Send the exception out. This done globally, so an exception
         // thrown from within a thread doesn't get lost.
         glogger_ptr->throwException(error.str());
      }
         break;

      //------------------------------------------------------------------------
      case Gem::Common::TERMINATION:
      {
         // Assemble the output string
         std::ostringstream error;
         error
            << std::endl
            << "================================================" << std::endl
            << "ERROR (recorded on " << boost::posix_time::second_clock::local_time() << ")" << std::endl
            << gm.getAccompInfo() << std::endl
            << std::endl
            << oss_.str()
            << std::endl
            << "If you suspect that this error is due to Geneva," << std::endl
            << "then please consider filing a bug via" << std::endl
            << "http://www.gemfony.com (link \"Bug Reports\") or" << std::endl
            << "through http://www.launchpad.net/geneva" << std::endl
            << std::endl
            << "We appreciate your help!" << std::endl
            << "The Geneva team" << std::endl
            << "================================================" << std::endl;

         // Do all necessary logging. We use a central exception file for this purpose.
         // The logger's routines will in addition try to print on the console. Note
         // that we ignore any "source" information, as this is already contained
         // in the logged message.
         GFileLogger gfl("GENEVA-TERMINATION.log");
         gfl.log(error.str());

         // Initiate the termination sequence.
         glogger_ptr->terminateApplication(error.str());
      }
         break;

      //------------------------------------------------------------------------
      case Gem::Common::WARNING:
      {
         // Assemble warning output
         std::ostringstream warning;
         warning
            << std::endl
            << "================================================" << std::endl
            << "WARNING (recorded on " << boost::posix_time::second_clock::local_time() << ")" << std::endl
            << gm.getAccompInfo() << std::endl
            << std::endl
            << oss_.str()
            << std::endl
            << "If you suspect that there is an underlying problem with Geneva," << std::endl
            << "then please consider filing a bug via" << std::endl
            << "http://www.gemfony.com (link \"Bug Reports\") or" << std::endl
            << "through http://www.launchpad.net/geneva" << std::endl
            << std::endl
            << "We appreciate your help!" << std::endl
            << "The Geneva team" << std::endl
            << "================================================" << std::endl;

         // Do all necessary logging.
         if(this->hasExtension()) {
            glogger_ptr->logWithSource(warning.str(), this->getExtension());
         } else {
            glogger_ptr->log(warning.str());
         }
      }
         break;

      //------------------------------------------------------------------------
      case Gem::Common::LOGGING:
      {
         // Do all necessary logging.
         if(this->hasExtension()) {
            glogger_ptr->logWithSource(oss_.str(), this->getExtension());
         } else {
            glogger_ptr->log(oss_.str());
         }
      }
         break;

      //------------------------------------------------------------------------
      case Gem::Common::FILE:
      {
         if(this->hasOneTimeLogFile()) {
            GFileLogger gfl(this->getOneTimeLogFile());
            gfl.log(oss_.str());
         } else {
            raiseException(
                  "In GLogStreamer::operator<<(const GManipulator&): Error!" << std::endl
                  << "Tried to log to one time log file without a file name specification" << std::endl
                  << "You need to supply a file name like this:" << std::endl
                  << "glogger(\"filename.txt\") << \"some log text\" << GFILE" << std::endl
            );
         }
      }
         break;

      //------------------------------------------------------------------------
      case Gem::Common::STDOUT:
      {
         glogger_ptr->toStdOut(oss_.str());
      }
         break;

      //------------------------------------------------------------------------
      case Gem::Common::STDERR:
      {
         glogger_ptr->toStdErr(oss_.str());
      }
         break;

      //------------------------------------------------------------------------
      default:
      {
         raiseException(
               "In GLogStreamer::operator<<(const GManipulator&): Error!" << std::endl
               << "Received invalid logging type " << gm.getLogType() << std::endl
         );
      }
      break;

      //------------------------------------------------------------------------
   };
}

/******************************************************************************/
/**
 * Returns the content of the ostringstream object.
 *
 * @return The content of the ostringstream object
 */
std::string GLogStreamer::content(void) const{
   return oss_.str();
}

/******************************************************************************/
/**
 * Stores an empty string in the ostringstream object.
 */
void GLogStreamer::reset(){
   oss_.str("");
}

/******************************************************************************/
/**
 * Checks whether an extension string has been registered
 */
bool GLogStreamer::hasExtension() const {
   return !extension_.empty();
}

/******************************************************************************/
/**
 * The content of the extension_ string
 */
std::string GLogStreamer::getExtension() const {
   return extension_;
}

/******************************************************************************/
/**
 * Checks whether a log file name has been registered
 */
bool GLogStreamer::hasOneTimeLogFile() const {
   return !logFile_.empty();
}

/******************************************************************************/
/**
 * The name of the manually specified file
 */
boost::filesystem::path GLogStreamer::getOneTimeLogFile() const {
   return logFile_;
}

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */
