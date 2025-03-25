BUCKET="generalbuckets-jx"

aws s3api list-multipart-uploads --bucket "$BUCKET" \
  --query 'Uploads[*].{Key:Key, UploadId:UploadId}' \
  --output json | \
  jq -r --arg BUCKET "$BUCKET" \
  '.[] | "aws s3api abort-multipart-upload --bucket \($BUCKET) --key \(.Key | @json) --upload-id \(.UploadId)"' | \
  sh

aws s3 rm s3://generalbuckets-jx --recursive