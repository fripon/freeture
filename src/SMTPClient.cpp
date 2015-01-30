/*
								SMTPClient.cpp

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file is part of:	freeture
*
*	Copyright:		(C) 2014-2015 Yoan Audureau
*                               FRIPON-GEOPS-UPSUD-CNRS
*
*	License:		GNU General Public License
*
*	FreeTure is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*	FreeTure is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*	You should have received a copy of the GNU General Public License
*	along with FreeTure. If not, see <http://www.gnu.org/licenses/>.
*
*	Last modified:		26/11/2014
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

void SMTPClient::getServerResponse(string request){

	size_t requestLength = strlen(request.c_str());
	char reply[1024];
	size_t reply_length = boost::asio::read(socket, boost::asio::buffer(reply, requestLength));
	cout << "Reply is: ";
	cout.write(reply, reply_length);
	cout << "\n";

}

bool SMTPClient::checkSMTPAnswer(const std::string & responseWaited, boost::asio::ip::tcp::socket & socket){

    bool res = true;

	//http://www.boost.org/doc/libs/1_40_0/doc/html/boost_asio/reference/streambuf.html
    boost::asio::streambuf response;
    std::string code;

	// Read data into a streambuf until it contains "\r\n".
    boost::asio::read_until(socket, response, "\r\n");
    {
        std::istream is(&response);
        is >> code;

        if( code != responseWaited)
        {
            std::cerr << "Not correct expecting answer from SMTP server : " << code << std::endl;
            res = false;

        }else{

			cout << "code : " <<code <<endl;

		}

		// Remove characters from response.
        response.consume( response.size() );

		// IO control command to get the amount of data that can be read without blocking.
        boost::asio::socket_base::bytes_readable command(true);
        socket.io_control(command);
        while( command.get() )
        {
            boost::asio::read_until(socket, response, "\r\n");
            socket.io_control(command);

        }
        response.consume( response.size() );

    }

    return res;
}

void SMTPClient::write(string data, string expectedAnswer, bool checkAnswer, bool printCmd){

	if(printCmd)
		cout << data << endl;

	boost::asio::write(socket, boost::asio::buffer(data));

	if(checkAnswer)
		checkSMTPAnswer(expectedAnswer, socket);

}

void SMTPClient::smtpServerConnection(){

	boost::asio::ip::tcp::resolver resolver(io_service);

	// Erreur par défaut.
	boost::system::error_code error = boost::asio::error::host_not_found;

    boost::asio::ip::tcp::resolver::query query(mailSmtpServer, "25");

    boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    boost::asio::ip::tcp::resolver::iterator end;

    // Pour chaque serveur trouvé.
    while(error && endpoint_iterator != end )
    {
        // On essaye de se connecter au serveur.
        socket.close();
        socket.connect(*endpoint_iterator++, error);
    }

	// Si aucun serveur n'a été trouvé.
	if( error ){
		cout << "aucun serveur de trouver" <<endl;
		throw boost::system::system_error(error);
	}

	checkSMTPAnswer("220", socket);

}

string SMTPClient::message(){

	// Final data to send.
	string message;

	// In case where mail client doesn't support HTML.
	string rawMessage = mailMessage;

	// Used to separate different mail formats.
	string section = "08zs01293eraf47a7804dcd17b1e";

	// Message using HTML.
	string htmlMessage =	"<html>\
								<body>\
									<p> " + mailMessage + " </p> ";

			if(imageInline)

				for(int i=0; i<mailAttachments.size(); i++){

					htmlMessage+=  "<img src=\"cid:image" + Conversion::intToString(i) + "@here\" alt=\"image\">";

				}

					htmlMessage+= "</body>\
							 </html>";

	// Specify the MIME version used.
	message = "Mime-Version: 1.0\r\n";

	// Head of the message starting by the Sender.
	message += "from: no-reply<" + mailFrom + ">\r\n";

	// Recipients.
	for(int i = 0; i < mailTo.size(); i++)
		message += "To: <" + mailTo.at(i) + ">\r\n";

	// Subject.
	message += "subject: " + mailSubject + "\r\n";

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

				// IMAGE inline.
				if(imageInline){

					for(int i=0; i<mailAttachments.size(); i++){

						cout <<  mailAttachments.at(i) << " -> " << endl;

						std::string s = mailAttachments.at(i);
						std::string delimiter = "/";
						vector<string> elements;
						string fileName;
						string fileExtension;

						size_t pos = 0;
						std::string token;
						while ((pos = s.find(delimiter)) != std::string::npos) {
							token = s.substr(0, pos);
							elements.push_back(token);
							s.erase(0, pos + delimiter.length());
						}
						elements.push_back(s);

						fileName = elements.back();
						cout << "fileName : " << fileName << endl;

						s = mailAttachments.at(i);
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
						cout << "fileExtension : " <<fileExtension << endl;


						message += "Content-Type: image/" + fileExtension + "; name =\"" + fileName + "\"\r\n";
						message += "Content-Transfer-Encoding: Base64\r\n";
						message += "Content-Disposition: inline\r\n";
						message += "Content-ID: <image" + Conversion::intToString(i) + "@here>\r\n\n"; // ID used in the construction of the HTML message above.
						message += "filename=\"" + fileName + "\"\r\n";

						string img =  ManageFiles::get_file_contents(mailAttachments.at(i).c_str());
						cout << "encode"<<endl;
						message += Base64::encodeBase64(img);

						message += "\r\n";

						message += "\r\n--" + section  + "\r\n";

					}
				}

		// ATTACHMENTS.

		// .txt attachment.
		message += "\r\n--" + section  + "\r\n";

		message += "Content-Type: text/plain; name =\"test.txt\"\r\n";
		message += "Content-Disposition: attachment\r\n";
		message += "filename=\"test.txt\"\r\n";

		message += "this is the attachment text\r\n";

		// png attachment.
		//http://dataurl.net/#dataurlmaker

		if(!imageInline){

			for(int i=0; i<mailAttachments.size(); i++){

				message += "\r\n--" + section  + "\r\n";

				/*
				//inverser string
				std::string inversePath = "";
				for (std::string::reverse_iterator rit=mailAttachments.at(i).rbegin(); rit!=mailAttachments.at(i).rend(); ++rit)
					inversePath += *rit;
				*/

				std::string s = mailAttachments.at(i);
				std::string delimiter = "/";
				vector<string> elements;
				string fileName;
				string fileExtension;

				size_t pos = 0;
				std::string token;
				while ((pos = s.find(delimiter)) != std::string::npos) {
					token = s.substr(0, pos);
					elements.push_back(token);
					s.erase(0, pos + delimiter.length());
				}
				elements.push_back(s);

				fileName = elements.back();
				cout << fileName << endl;

				s = mailAttachments.at(i);
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
				cout << fileExtension << endl;

				message += "Content-Type: image/" + fileExtension + "; name =\"" + fileName + "\"\r\n";
				message += "Content-Transfer-Encoding: Base64\r\n";
				message += "Content-Disposition: attachment\r\n";
				message += "filename=\"" + fileName + "\"\r\n\n";

				string img =  ManageFiles::get_file_contents(mailAttachments.at(i).c_str());

				message += Base64::encodeBase64(img);


				message += "\r\n";

			}
		}

	// Mail end.
	message += "\r\n--" + section  + "--\r\n";

	return message;

}

void SMTPClient::send(	string from,
			vector<string> to,
			string subject,
			string msg,
			vector<string> pathAttachments,
			bool imgInline){

	string data;


	mailTo			= to;
	mailFrom		= from;
	mailSubject		= subject;
	mailMessage		= msg;
	mailSubject		= subject;
	mailAttachments = pathAttachments;
	imageInline		= imgInline;

	// Connection to SMTP server.
	smtpServerConnection();

	// HELO to SMTP server.
	write("HELO " + mailServerHostname + "\r\n", "250", true, true);

	// Sender.
	write("MAIL FROM: <" + mailFrom + ">\r\n", "250", true, true);

	// Recipients.
	for(int i = 0; i < mailTo.size(); i++)
		write("RCPT TO: <" + mailTo.at(i) + ">\r\n", "250", true, true);

	// Start to sending data.
	write("DATA\r\n", "354", true, true);

	// Build message using MIME.
	data = message();

	// Send data.
	write(data, "", false, false);

	// End of sending data.
	write("\r\n.\r\n", "250", true, true);

	// Deconnection.
	write("QUIT\r\n", "221", true, true);

}
