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


const char *urlbase = "https://www.googleapis.com/youtube/v3/search?part=snippet";

const char *reply = R"XXX({
 "kind": "youtube#searchListResponse",
 "etag": "\"ag-oqvH8dumDXQP6JcFz5Tsa_OA/iK1JRGWv4_0okQDtI6-eU0b2rVc\"",
 "nextPageToken": "CAUQAA",
 "pageInfo": {
  "totalResults": 1000000,
  "resultsPerPage": 5
 },
 "items": [
  {
   "kind": "youtube#searchResult",
   "etag": "\"ag-oqvH8dumDXQP6JcFz5Tsa_OA/TjjO38dXPQzxaSgoCvxyjTvELeI\"",
   "id": {
    "kind": "youtube#video",
    "videoId": "O-Q6lowLKXY"
   },
   "snippet": {
    "publishedAt": "2014-04-27T22:58:04.000Z",
    "channelId": "UCtxfV5XAFXQ7AE7vbUw6jOQ",
    "title": "Santa Barbara Gauchos vs Arizona Lax Cats | SLC Playoffs presented by Total Lacrosse",
    "description": "Subscribe to The Lacrosse Network for more great lacrosse content! http://www.youtube.com/subscription_center?add_user=TheLacrosseNetwork ...",
    "thumbnails": {
     "default": {
      "url": "https://i.ytimg.com/vi/O-Q6lowLKXY/default.jpg"
     },
     "medium": {
      "url": "https://i.ytimg.com/vi/O-Q6lowLKXY/mqdefault.jpg"
     },
     "high": {
      "url": "https://i.ytimg.com/vi/O-Q6lowLKXY/hqdefault.jpg"
     }
    },
    "channelTitle": "TheLacrosseNetwork",
    "liveBroadcastContent": "none"
   }
  },
  {
   "kind": "youtube#searchResult",
   "etag": "\"ag-oqvH8dumDXQP6JcFz5Tsa_OA/txATIiGtn_rgN91T0TBIDgncn5A\"",
   "id": {
    "kind": "youtube#video",
    "videoId": "kPoQFzOOfPw"
   },
   "snippet": {
    "publishedAt": "2014-04-26T11:48:47.000Z",
    "channelId": "UCo2pAlAg46p3eVUX_lNHOKA",
    "title": "Funny Videos - Fail Compilation, Funny Pranks and Funny Cats Videos | New Funny Video",
    "description": "Funny Videos Funny Videos - Fails Compilation, Funny Pranks and Funny Animals Videos | New Funny Videos Don't forget to subscribe :) Funny Videos Funny ...",
    "thumbnails": {
     "default": {
      "url": "https://i.ytimg.com/vi/kPoQFzOOfPw/default.jpg"
     },
     "medium": {
      "url": "https://i.ytimg.com/vi/kPoQFzOOfPw/mqdefault.jpg"
     },
     "high": {
      "url": "https://i.ytimg.com/vi/kPoQFzOOfPw/hqdefault.jpg"
     }
    },
    "channelTitle": "AnimeAllTheTimeHD",
    "liveBroadcastContent": "none"
   }
  },
  {
   "kind": "youtube#searchResult",
   "etag": "\"ag-oqvH8dumDXQP6JcFz5Tsa_OA/0bNA7HaHYXw0q7Tc7rglmikK1_s\"",
   "id": {
    "kind": "youtube#video",
    "videoId": "n_MSn2-mi6A"
   },
   "snippet": {
    "publishedAt": "2014-04-27T10:32:04.000Z",
    "channelId": "UCSszrpGLUBZkS2hDkvggQKg",
    "title": "Power v Cats highlights - Round 6, 2014",
    "description": "Watch highlights from the Power's big win over the Cats in front of a record non-Showdown crowd at Adelaide Oval.",
    "thumbnails": {
     "default": {
      "url": "https://i.ytimg.com/vi/n_MSn2-mi6A/default.jpg"
     },
     "medium": {
      "url": "https://i.ytimg.com/vi/n_MSn2-mi6A/mqdefault.jpg"
     },
     "high": {
      "url": "https://i.ytimg.com/vi/n_MSn2-mi6A/hqdefault.jpg"
     }
    },
    "channelTitle": "PortAdelaideFC",
    "liveBroadcastContent": "none"
   }
  },
  {
   "kind": "youtube#searchResult",
   "etag": "\"ag-oqvH8dumDXQP6JcFz5Tsa_OA/mSWfFtsEz5Rufro1iZYppXESApE\"",
   "id": {
    "kind": "youtube#video",
    "videoId": "0gJTCZgHw9A"
   },
   "snippet": {
    "publishedAt": "2014-04-26T00:54:46.000Z",
    "channelId": "UC-qEoO7dgas5RcikKyds7iA",
    "title": "Funny Cats Compilation ~ Cute Cat Videos ~ Best Fail Kittens Compilations",
    "description": "Hi, thanks for visiting my channel Like, Subscribe and Comment your favourite cat More Cats Videos go here http://goo.gl/CZQpTf Top Funny Cats Compilation ...",
    "thumbnails": {
     "default": {
      "url": "https://i.ytimg.com/vi/0gJTCZgHw9A/default.jpg"
     },
     "medium": {
      "url": "https://i.ytimg.com/vi/0gJTCZgHw9A/mqdefault.jpg"
     },
     "high": {
      "url": "https://i.ytimg.com/vi/0gJTCZgHw9A/hqdefault.jpg"
     }
    },
    "channelTitle": "TheFunnyCuteCats",
    "liveBroadcastContent": "none"
   }
  },
  {
   "kind": "youtube#searchResult",
   "etag": "\"ag-oqvH8dumDXQP6JcFz5Tsa_OA/wOm_6XB4yk2Yw_J2lkqa6mPKf-8\"",
   "id": {
    "kind": "youtube#video",
    "videoId": "S7b1aLY3tyI"
   },
   "snippet": {
    "publishedAt": "2014-03-12T14:00:03.000Z",
    "channelId": "UCaIM6y9YvfUb_IxS2reKsqw",
    "title": "Funny Cats Compilation 2014 | 20 min",
    "description": "Subscribe: - https://www.youtube.com/user/ptichkalastochka?sub_confirmation=1 • Follow me on facebook, it's the cool thing to do these days ...",
    "thumbnails": {
     "default": {
      "url": "https://i.ytimg.com/vi/S7b1aLY3tyI/default.jpg"
     },
     "medium": {
      "url": "https://i.ytimg.com/vi/S7b1aLY3tyI/mqdefault.jpg"
     },
     "high": {
      "url": "https://i.ytimg.com/vi/S7b1aLY3tyI/hqdefault.jpg"
     }
    },
    "channelTitle": "PtichkaLastochka",
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

YoutubeDownloader::YoutubeDownloader(const std::string &s) : p(new Private(s)) {

}

YoutubeDownloader::~YoutubeDownloader() {
    delete p;
}

std::vector<YoutubeResult> YoutubeDownloader::query(const std::string &q) {
    std::string output(reply);//= p->download(q);
    return parse_json(output);
}
