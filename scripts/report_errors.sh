#!/usr/bin/env bash

export ERROR_LOG="/tmp/error.log"

function report_error_to_github
{
    if [ $? -eq 0 ]
    then
        exit 0
    fi

    if [ -z $CIRCLE_PR_NUMBER ]
    then
        CIRCLE_PR_NUMBER="${CIRCLE_PULL_REQUEST//[^0-9]/}"
    fi

    ERROR_MSG=$(cat $ERROR_LOG)

    echo $ERROR_MSG

    if [ ! -z $CI ]
    then
        echo "posting error message to github"
        post_error_to_github
    fi
}

function post_error_to_github
{
    if [ -z $CIRCLE_PR_NUMBER ]
    then
        CIRCLE_PR_NUMBER="${CIRCLE_PULL_REQUEST//[^0-9]/}"
    fi

    GITHUB_API_URL="https://api.github.com/repos/$CIRCLE_PROJECT_USERNAME/$CIRCLE_PROJECT_REPONAME/issues/$CIRCLE_PR_NUMBER/comments"

    ESCAPED_ERROR_MSG=$(cat -e $ERROR_LOG | sed 's/\\/\\\\/g' | sed 's/"/\\\"/g')

    FORMATTED_ERROR_MSG=$(echo $ESCAPED_ERROR_MSG | sed 's/\$/\\n/g' | tr -d '\n')

    curl --request POST \
      --url $GITHUB_API_URL \
      --header 'accept: application/vnd.github.v3+json' \
      --header 'content-type: application/json' \
      -u stackenbotten:$GITHUB_ACCESS_TOKEN \
      --data "{\"body\": \"There was an error when running \`$CIRCLE_JOB\` for commit \`$CIRCLE_SHA1\`:\n\`\`\`\n$FORMATTED_ERROR_MSG\n\`\`\`\nPlease check that your changes are working as intended.\"}"
}

trap report_error_to_github EXIT
