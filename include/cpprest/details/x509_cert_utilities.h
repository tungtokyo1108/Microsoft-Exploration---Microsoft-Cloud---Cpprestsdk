/*
 * x509_cert_utilities.h
 *
 *  Created on: Nov 6, 2018
 *      Student (MIG Virtual Developer): Tung Dang
 */

#ifndef INCLUDE_CPPREST_DETAILS_X509_CERT_UTILITIES_H_
#define INCLUDE_CPPREST_DETAILS_X509_CERT_UTILITIES_H_

#include <string>

#if defined(__APPLE__) || (defined(ANDROID) || defined(__ANDROID__)) || (defined(_WIN32)  && !defined(__cplusplus_winrt) && !defined(_M_ARM) && !defined(CPPREST_EXCLUDE_WEBSOCKETS))

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4005)
#endif
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif
#include <boost/asio/ssl.hpp>
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
namespace web { namespace http { namespace client { namespace details {
bool verify_cert_chain_platform_specific(boost::asio::ssl::verify_context &verifyCtx, const std::string &hostName);

}}}}
}
#endif

#endif /* INCLUDE_CPPREST_DETAILS_X509_CERT_UTILITIES_H_ */
