import Ubuntu.OnlineAccounts.Plugin 1.0

OAuthMain {

    // Override creationComponent so we can retreive that account's username
    // This then appears in the accounts management page
    creationComponent: OAuth {
        function completeCreation(reply) {
            console.log("Vimeo got reply: " + reply.AccessToken)
            var http = new XMLHttpRequest()
            var url = "https://api.vimeo.com/me";
            http.open("GET", url, true);
            http.setRequestHeader("Authorization", "Bearer " + reply.AccessToken)
            http.setRequestHeader("Accept", "application/vnd.vimeo.*+json; version=3.0")
            http.setRequestHeader("User-Agent", "unity-scope-vimeo 0.1; (http: //developer.vimeo.com/api/docs)")
            http.onreadystatechange = function() {
                if (http.readyState === 4) {
                    if (http.status == 200) {
                        console.log("ok")
                        var response = JSON.parse(http.responseText)
                        console.log("got name: " + response.name)
                        account.updateDisplayName(response.name)
                        account.synced.connect(finished)
                        account.sync()
                    } else {
                        console.log("error: " + http.responseText)
                        cancel()
                    }
                }
            };

            http.send(null);
        }
    }
}
