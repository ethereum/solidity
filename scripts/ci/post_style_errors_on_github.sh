#!/usr/bin/env bash

ERROR_LOG="$1"

function report_error_to_github
{
    [[ $CIRCLE_PR_NUMBER != "" ]] || CIRCLE_PR_NUMBER="${CIRCLE_PULL_REQUEST//[^0-9]/}"

    if [[ $CI == "true" ]]
    then
        echo "posting error message to github"
        post_error_to_github
        post_review_comment_to_github
    fi
}

function post_error_to_github
{
    GITHUB_API_URL="https://api.github.com/repos/$CIRCLE_PROJECT_USERNAME/$CIRCLE_PROJECT_REPONAME/issues/$CIRCLE_PR_NUMBER/comments"

    ESCAPED_ERROR_MSG=$(cat -e "$ERROR_LOG" | sed 's/\\/\\\\/g' | sed 's/"/\\\"/g')

    FORMATTED_ERROR_MSG=$(echo "$ESCAPED_ERROR_MSG" | sed 's/\$/\\n/g' | tr -d '\n')

    curl --request POST \
        --url "$GITHUB_API_URL" \
        --header 'accept: application/vnd.github.v3+json' \
        --header 'content-type: application/json' \
        -u "stackenbotten:$GITHUB_ACCESS_TOKEN" \
        --data "{\"body\": \"There was an error when running \`$CIRCLE_JOB\` for commit \`$CIRCLE_SHA1\`:\n\`\`\`\n$FORMATTED_ERROR_MSG\n\`\`\`\nPlease check that your changes are working as intended.\"}"
}

function post_review_comment_to_github
{
    GITHUB_API_URL="https://api.github.com/repos/$CIRCLE_PROJECT_USERNAME/$CIRCLE_PROJECT_REPONAME/pulls/$CIRCLE_PR_NUMBER/comments"

    sed -i 1d "$ERROR_LOG"

    while read -r line
    do
        ERROR_PATH=$(echo "$line" | grep -oE ".*\.cpp")
        ERROR_LINE=$(echo "$line" | grep -oE "[0-9]*")
        [[ $ERROR_PATH != "" ]] || { echo "ERROR: Error message does not contain file path."; exit 1; }
        [[ $ERROR_LINE != "" ]] || { echo "ERROR: Error message does not contain line number."; exit 1; }

        curl --request POST \
            --url "$GITHUB_API_URL" \
            --header 'accept: application/vnd.github.v3+json, application/vnd.github.comfort-fade-preview+json' \
            --header 'content-type: application/json' \
            -u "stackenbotten:$GITHUB_ACCESS_TOKEN" \
            --data "{\"commit_id\": \"$CIRCLE_SHA1\", \"path\": \"$ERROR_PATH\", \"line\": $ERROR_LINE, \"side\": \"RIGHT\", \"body\": \"Coding style error\"}"
    done < "$ERROR_LOG"
}

report_error_to_github
