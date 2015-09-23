/*
                                SMTPClient.cpp

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*   This file is part of:   freeture
*
*   Copyright:      (C) 2014-2015 Yoan Audureau
*                               FRIPON-GEOPS-UPSUD-CNRS
*
*   License:        GNU General Public License
*
*   FreeTure is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*   FreeTure is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*   You should have received a copy of the GNU General Public License
*   along with FreeTure. If not, see <http://www.gnu.org/licenses/>.
*
*   Last modified:      20/07/2015
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/**
* \file    SMTPClient.cpp
* \author  Yoan Audureau -- FRIPON-GEOPS-UPSUD
* \version 1.0
* \date    03/12/2014
* \brief   SMTP connection and send mails.
*/

#include "SMTPClient.h"

boost::log::sources::severity_logger< LogSeverityLevel >  SMTPClient::logger;

SMTPClient::Init SMTPClient::initializer;

bool SMTPClient::checkSMTPAnswer(const std::string & responseWaited, boost::asio::ip::tcp::socket & socket) {

    bool res = true;

    //http://www.boost.org/doc/libs/1_40_0/doc/html/boost_asio/reference/streambuf.html
    boost::asio::streambuf response;
    std::string code;

    // Read data into a streambuf until it contains "\r\n".
    boost::asio::read_until(socket, response, "\r\n");

    {
        std::istream is(&response);
        is >> code;

        if(code != responseWaited){

            std::cerr << "Not correct expecting answer from SMTP server : " << code << std::endl;
            res = false;

        }else{

            //cout << "code : " <<code <<endl;
            BOOST_LOG_SEV(logger,notification) << "code :" << code;

        }

        // Remove characters from response.
        response.consume(response.size());

        // IO control command to get the amount of data that can be read without blocking.
        boost::asio::socket_base::bytes_readable command(true);
        socket.io_control(command);

        while(command.get()) {

            boost::asio::read_until(socket, response, "\r\n");
            socket.io_control(command);

        }

        response.consume( response.size() );

    }

    return res;
}

void SMTPClient::write(string data, string expectedAnswer, bool checkAnswer, boost::asio::ip::tcp::socket & socket) {

    boost::asio::write(socket, boost::asio::buffer(data));

    if(checkAnswer)
        checkSMTPAnswer(expectedAnswer, socket);

}

bool SMTPClient::getFileContents(const char *filename, string &content){

    ifstream in(filename, ios::in | ios::binary);
    cout << filename<< endl;
    if(in){

        in.seekg(0, ios::end);
        content.resize(in.tellg());
        in.seekg(0, ios::beg);
        in.read(&content[0], content.size());
        in.close();
        return true;

    }

    return false;

}


string SMTPClient::buildMessage( string msg, vector<string> mMailAttachments,
                                 vector<string> mMailTo,  string mMailFrom,  string mMailSubject){

    // Final data to send.
    string message;

    // In case where mail client doesn't support HTML.
    string rawMessage = msg;

    // Used to separate different mail formats.
    string section = "08zs01293eraf47a7804dcd17b1e";

    // Message using HTML.
    string htmlMessage =    "<html>\
                                <body>\
                                    <p> " + msg + " </p> ";
                 htmlMessage+= "</body>\
                             </html>";

    // Specify the MIME version used.
    message = "Mime-Version: 1.0\r\n";

    // Head of the message starting by the Sender.
    message += "from: no-reply<" + mMailFrom + ">\r\n";

    // Recipients.
    for(int i = 0; i < mMailTo.size(); i++)
        message += "To: <" + mMailTo.at(i) + ">\r\n";

    // Subject.
    message += "subject: " + mMailSubject + "\r\n";

    /*

    MAIL STRUCTURE MODEL :

    multipart/mixed
        multipart/alternative
            text/plain
            multipart/related
                text/html
                image/jpg
        some/thing (disposition:attachment)
        some/thing (disposition:attachment)

    */

    message += "Content-Type: multipart/mixed; boundary=" + section  + "\r\n";
    message += "\r\n";

        message += "Content-Type: multipart/alternative; boundary=" + section + "\r\n";
        message += "\r\n";

            // Raw text.
            message += "\r\n--" + section  + "\r\n";

            message += "Content-type: text/plain; charset=ISO-8859-1\r\n";
            message += "\r\n";
            message += rawMessage;
            message += "\r\n";

            message += "\r\n--" + section  + "\r\n";

            message += "Content-Type: multipart/related; boundary=" + section  + "\r\n";
            message += "\r\n";

                // HTML text.
                message += "--" + section  + "\r\n";
                message += "Content-type: text/html; charset=ISO-8859-1\r\n";
                message += "\r\n";
                message += htmlMessage ;
                message += "\r\n";

                message += "\r\n--" + section  + "\r\n";

        // ATTACHMENTS.

        // .txt attachment.
        /*message += "\r\n--" + section  + "\r\n";

        message += "Content-Type: text/plain; name =\"test.txt\"\r\n";
        message += "Content-Disposition: attachment\r\n";
        message += "filename=\"test.txt\"\r\n";

        message += "this is the attachment text\r\n";*/

        // png attachment.
        //http://dataurl.net/#dataurlmaker

        for(int i=0; i<mMailAttachments.size(); i++){

            message += "\r\n--" + section  + "\r\n";

            std::string s = mMailAttachments.at(i);
            std::string delimiter = "/";

            vector<string> elements;
            string fileName;
            string fileExtension;

            size_t pos = 0;
            std::string token;
            while((pos = s.find(delimiter)) != std::string::npos) {

                token = s.substr(0, pos);
                elements.push_back(token);
                //cout << token << endl;
                s.erase(0, pos + delimiter.length());

            }

            elements.push_back(s);

            fileName = elements.back();
            //cout << fileName << endl;

            s = mMailAttachments.at(i);
            delimiter = ".";
            elements.clear();

            pos = 0;
            token="";
            while ((pos = s.find(delimiter)) != std::string::npos) {
                token = s.substr(0, pos);
                elements.push_back(token);
                s.erase(0, pos + delimiter.length());
            }
            elements.push_back(s);

            fileExtension = elements.back();
            //  cout << fileExtension << endl;

            message += "Content-Type: image/" + fileExtension + "; name =\"" + fileName + "\"\r\n";
            message += "Content-Transfer-Encoding: Base64\r\n";
            message += "Content-Disposition: attachment\r\n";
            message += "filename=\"" + fileName + "\"\r\n\n";
            // cout << "getFileContents : " << mMailAttachments.at(i)<< endl;
            string img;
            if(!getFileContents(mMailAttachments.at(i).c_str(), img)) {
                cout << "Fail to load image to attach to mail message" << endl;
                BOOST_LOG_SEV(logger,fail) << "Fail to load image to attach to mail message";
                img = "";
            }
            //  cout << "end getFileContents" << endl;
            message += Base64::encodeBase64(img);

            message += "\r\n";

        }

    // Mail end.
    message += "\r\n--" + section  + "--\r\n";

    return message;

}

void SMTPClient::sendMail(  string            server,
                            string            login,
                            string            password,
                            string            from,
                            vector<string>    to,
                            string            subject,
                            string            message,
                            vector<string>    pathAttachments,
                            SmtpSecurity      securityType) {

    try {

        switch(securityType) {

            case NO_SECURITY :

                {

                    Socket socket(server, 25);
                    checkSMTPAnswer("220", *socket.GetSocket());

                    // HELO to SMTP server.
                    BOOST_LOG_SEV(logger,normal) << "HELLO to SMTP server.";
                    write("HELO " + server + "\r\n", "250", true, *socket.GetSocket());

                    // Sender.
                    BOOST_LOG_SEV(logger,normal) << "Write sender.";
                    write("MAIL FROM: <" + from + ">\r\n", "250", true, *socket.GetSocket());

                    // Recipients.
                    BOOST_LOG_SEV(logger,normal) << "Write recipients.";
                    for(int i = 0; i < to.size(); i++)
                    write("RCPT TO: <" + to.at(i) + ">\r\n", "250", true, *socket.GetSocket());

                    // Start to sending data.
                    BOOST_LOG_SEV(logger,normal) << "Write datas.";
                    write("DATA\r\n", "354", true, *socket.GetSocket());

                    // Build message using MIME.
                    BOOST_LOG_SEV(logger,normal) << "Build message using MIME.";
                    string data = buildMessage(message, pathAttachments, to, from, subject);

                    // Send data.
                    BOOST_LOG_SEV(logger,normal) << "Send data.";
                    write(data, "", false, *socket.GetSocket());

                    // End of sending data.
                    BOOST_LOG_SEV(logger,normal) << "End of sending data.";
                    write("\r\n.\r\n", "250", true, *socket.GetSocket());

                    // Deconnection.
                    BOOST_LOG_SEV(logger,normal) << "Deconnection.";
                    write("QUIT\r\n", "221", true, *socket.GetSocket());

                    BOOST_LOG_SEV(logger,notification) << "Mail sent.";

                }

                break;

            case USE_SSL :

                {

                    Socket socket(server, 465);

                    static const string newline = "\r\n";

                    BOOST_LOG_SEV(logger,notification) << "Initialize SSL connection.";
                    OpenSSL::StaticInitialize sslInitializer;

                    OpenSSL openSSL(socket.GetSocket()->native());
                    BOOST_LOG_SEV(logger,normal) << openSSL.Read(ReceiveFunctor(220));

                    BOOST_LOG_SEV(logger,notification) << string("EHLO ") << server;
                    openSSL.Write(string("EHLO ") + server + newline);
                    BOOST_LOG_SEV(logger,normal) << openSSL.Read(ReceiveFunctor(250));

                    BOOST_LOG_SEV(logger,notification) << "AUTH LOGIN";
                    openSSL.Write(string("AUTH LOGIN") + newline);
                    BOOST_LOG_SEV(logger,normal) << openSSL.Read(ReceiveFunctor(334));

                    BOOST_LOG_SEV(logger,notification) << "Write Login";
                    openSSL.Write(Base64::encodeBase64(login) + newline);
                    BOOST_LOG_SEV(logger,normal) << openSSL.Read(ReceiveFunctor(334));

                    BOOST_LOG_SEV(logger,notification) << "Write password";
                    openSSL.Write(password + newline);
                    BOOST_LOG_SEV(logger,normal) << openSSL.Read(ReceiveFunctor(235));

                    BOOST_LOG_SEV(logger,notification) << "MAIL FROM:<" << from << ">";
                    openSSL.Write(string("MAIL FROM:<") + from + ">" + newline);
                    BOOST_LOG_SEV(logger,normal) << openSSL.Read(ReceiveFunctor(250));

                    for(int i = 0; i < to.size(); i++) {

                        BOOST_LOG_SEV(logger,notification) << "RCPT TO:<" << to.at(i) << ">";
                        openSSL.Write(string("RCPT TO:<") + to.at(i) + ">" + newline);
                        BOOST_LOG_SEV(logger,normal) << openSSL.Read(ReceiveFunctor(250));

                    }

                    BOOST_LOG_SEV(logger,notification) << "DATA";
                    openSSL.Write(string("DATA") + newline);
                    BOOST_LOG_SEV(logger,normal) << openSSL.Read(ReceiveFunctor(354));

                    BOOST_LOG_SEV(logger,notification) << "Build message";
                    string m = buildMessage(message, pathAttachments, to, from, subject);
                    openSSL.Write( m + newline + "." + newline);
                    //BOOST_LOG_SEV(logger,normal) << openSSL.Read(ReceiveFunctor(250));

                    BOOST_LOG_SEV(logger,notification) << "QUIT";
                    openSSL.Write(string("QUIT: ") + newline);

                    BOOST_LOG_SEV(logger,notification) << "Mail sent.";

                }

                break;

            case USE_TLS :

                break;

        }

    }catch(exception& e){

        BOOST_LOG_SEV(logger, critical) << e.what();

    }catch(const char * msg){

        BOOST_LOG_SEV(logger,fail) << "Fail to send mail : " << msg;

    }
}
