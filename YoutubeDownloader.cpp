/*
 * Copyright (C) 2014 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by Jussi Pakkanen <jussi.pakkanen@canonical.com>
 *
 */

#include"YoutubeDownloader.h"
#include<iostream>
#include<memory>
#include<libsoup/soup.h>
#include<json/json.h>

const char *urlbase = "https://www.googleapis.com/youtube/v3/search?part=snippet";

const char *reply = R"XXX({
 "kind": "youtube#searchListResponse",
 "etag": "\"OilNCqKLXpFjeQ5CI-_BZqeMuCo/1MHT_ZCRHRM3vhrbPoOk-O_yh3Q\"",
 "nextPageToken": "CAUQAA",
 "pageInfo": {
  "totalResults": 1000000,
  "resultsPerPage": 5
 },
 "items": [
  {
   "kind": "youtube#searchResult",
   "etag": "\"OilNCqKLXpFjeQ5CI-_BZqeMuCo/v3i40y19aP3qQOyhF0d_QOCfMgY\"",
   "id": {
    "kind": "youtube#video",
    "videoId": "9bZkp7q19f0"
   },
   "snippet": {
    "publishedAt": "2012-07-15T07:46:32.000Z",
    "channelId": "UCrDkAvwZum-UTjHmzDI2iIw",
    "title": "PSY - GANGNAM STYLE (강남스타일) M/V",
    "description": "PSY - Gangnam Style (강남스타일) ▷ NOW available on iTunes: http://Smarturl.it/psygangnam ▷ Official PSY Online Store US & International ...",
    "thumbnails": {
     "default": {
      "url": "https://i.ytimg.com/vi/9bZkp7q19f0/default.jpg"
     },
     "medium": {
      "url": "https://i.ytimg.com/vi/9bZkp7q19f0/mqdefault.jpg"
     },
     "high": {
      "url": "https://i.ytimg.com/vi/9bZkp7q19f0/hqdefault.jpg"
     }
    },
    "channelTitle": "officialpsy",
    "liveBroadcastContent": "none"
   }
  },
  {
   "kind": "youtube#searchResult",
   "etag": "\"OilNCqKLXpFjeQ5CI-_BZqeMuCo/Z0p5gx9qGJ0kjWWCm8ezz3y-jXc\"",
   "id": {
    "kind": "youtube#video",
    "videoId": "mIQToVqDMb8"
   },
   "snippet": {
    "publishedAt": "2012-12-12T23:03:05.000Z",
    "channelId": "UCl-12mHomEdglO3seXyvr5A",
    "title": "Gangnam Style Official Music Video - 2012 PSY with Oppan Lyrics & MP3 Download",
    "description": "Gangnam Style official music video -▷ http://MUSlCDOWNLOADS.com/mp3 ◁- ♫ -▷ http://Vid2Audio.net/ ◁- Convert Video to Audio ...",
    "thumbnails": {
     "default": {
      "url": "https://i.ytimg.com/vi/mIQToVqDMb8/default.jpg"
     },
     "medium": {
      "url": "https://i.ytimg.com/vi/mIQToVqDMb8/mqdefault.jpg"
     },
     "high": {
      "url": "https://i.ytimg.com/vi/mIQToVqDMb8/hqdefault.jpg"
     }
    },
    "channelTitle": "kaboomtribe",
    "liveBroadcastContent": "none"
   }
  },
  {
   "kind": "youtube#searchResult",
   "etag": "\"OilNCqKLXpFjeQ5CI-_BZqeMuCo/hOYw1h4rRuP9g_ICakZ1SN8hUms\"",
   "id": {
    "kind": "youtube#channel",
    "channelId": "UCrDkAvwZum-UTjHmzDI2iIw"
   },
   "snippet": {
    "publishedAt": "2010-10-04T02:50:53.000Z",
    "channelId": "UCrDkAvwZum-UTjHmzDI2iIw",
    "title": "officialpsy",
    "description": "PSY Official YouTube Channel.",
    "thumbnails": {
     "default": {
      "url": "https://lh3.googleusercontent.com/-0Xgl841SU7Y/AAAAAAAAAAI/AAAAAAAAAAA/_bKTxRDm1kw/photo.jpg"
     },
     "medium": {
      "url": "https://lh3.googleusercontent.com/-0Xgl841SU7Y/AAAAAAAAAAI/AAAAAAAAAAA/_bKTxRDm1kw/photo.jpg"
     },
     "high": {
      "url": "https://lh3.googleusercontent.com/-0Xgl841SU7Y/AAAAAAAAAAI/AAAAAAAAAAA/_bKTxRDm1kw/photo.jpg"
     }
    },
    "channelTitle": "officialpsy",
    "liveBroadcastContent": "none"
   }
  },
  {
   "kind": "youtube#searchResult",
   "etag": "\"OilNCqKLXpFjeQ5CI-_BZqeMuCo/7hHcHtNryjNxZ0NKkgo9EUI7OvU\"",
   "id": {
    "kind": "youtube#video",
    "videoId": "60MQ3AG1c8o"
   },
   "snippet": {
    "publishedAt": "2012-07-16T15:14:04.000Z",
    "channelId": "UCcpIYPgJrK2GQBYaJBZk2EQ",
    "title": "[Live HD 720p] 120715 - PSY - Gangnam style (Comeback stage) - Inkigayo",
    "description": "Introducing all CapsuleHD Channels; Performances from Mnet (M Countdown, MUST) - http://www.youtube.com/CapsuleHD13 Performances from K (Music ...",
    "thumbnails": {
     "default": {
      "url": "https://i.ytimg.com/vi/60MQ3AG1c8o/default.jpg"
     },
     "medium": {
      "url": "https://i.ytimg.com/vi/60MQ3AG1c8o/mqdefault.jpg"
     },
     "high": {
      "url": "https://i.ytimg.com/vi/60MQ3AG1c8o/hqdefault.jpg"
     }
    },
    "channelTitle": "CapsuleHD20",
    "liveBroadcastContent": "none"
   }
  },
  {
   "kind": "youtube#searchResult",
   "etag": "\"OilNCqKLXpFjeQ5CI-_BZqeMuCo/_lB1QU40WAtuNI2Uk6gdykCBtws\"",
   "id": {
    "kind": "youtube#video",
    "videoId": "wcLNteez3c4"
   },
   "snippet": {
    "publishedAt": "2012-08-14T15:00:06.000Z",
    "channelId": "UCrDkAvwZum-UTjHmzDI2iIw",
    "title": "PSY (ft. HYUNA) 오빤 딱 내 스타일",
    "description": "6TH STUDIO ALBUM [PSY 6甲] ▷ NOW available on iTunes: http://smarturl.it/psy6gap1 ▷ Official PSY Online Store US & International ...",
    "thumbnails": {
     "default": {
      "url": "https://i.ytimg.com/vi/wcLNteez3c4/default.jpg"
     },
     "medium": {
      "url": "https://i.ytimg.com/vi/wcLNteez3c4/mqdefault.jpg"
     },
     "high": {
      "url": "https://i.ytimg.com/vi/wcLNteez3c4/hqdefault.jpg"
     }
    },
    "channelTitle": "officialpsy",
    "liveBroadcastContent": "none"
   }
  }
 ]
}
)XXX";

struct YoutubeDownloader::Private {
    std::string apikey;

    Private(const std::string &apikey) : apikey(apikey) {
    }
    std::string download(const std::string &q);
    void parse(const std::string &input);
};

std::string YoutubeDownloader::Private::download(const std::string &q) {
    std::unique_ptr<SoupSession, void(*)(gpointer)> session(soup_session_sync_new(), g_object_unref);
    if(!session) {
        throw std::runtime_error("Could not create Soup session.");
    }
    std::string url(urlbase);
    url += "&q=" + q; // FIXME: quote input
    url += "&key=" + apikey;
    std::unique_ptr<SoupMessage, void(*)(gpointer)> msg(soup_message_new("GET", url.c_str()), g_object_unref);
    std::cout << "Downloading url " << url << std::endl;
    guint status = soup_session_send_message(session.get(), msg.get());
    if(!SOUP_STATUS_IS_SUCCESSFUL(status)) {
        std::cerr << "Download failed." << std::endl;
    }
    std::string result(msg->response_body->data, msg->response_body->length);
    return result;
}

void YoutubeDownloader::Private::parse(const std::string &input) {
    Json::Value root;
    Json::Reader reader;
    if(not reader.parse(input, root)) {
        std::cerr << "Json parsing failed." << std::endl;
        return;
    }
    const Json::Value items = root["items"];
    for(unsigned int i=0; i<items.size(); i++) {
        const Json::Value snippet = items[i]["snippet"];
        std::string title = snippet["title"].asString();
        std::string description = snippet["description"].asString();
        std::string screenshot = snippet["thumbnails"]["high"]["url"].asString();
        std::cout << title << "\n " << description << "\n " << screenshot << std::endl;
    }
}

YoutubeDownloader::YoutubeDownloader(const std::string &s) : p(new Private(s)) {

}

YoutubeDownloader::~YoutubeDownloader() {
    delete p;
}

void YoutubeDownloader::query(const std::string &q) {
    std::string output(reply);// = p->download(q);
    p->parse(output);
}
