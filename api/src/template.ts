export function prepare(meta: {
  title?: string,
  description?: string,
  keywords?: string,
  author?: string
}, custom: any = {}, session: any = {
  logged: false
}): any
{
  return Object.assign({
    title: meta.title ? meta.title : 'No title',
    description: meta.description ? meta.description : 'No description',
    keywords: meta.keywords ? meta.keywords : 'unknown,page',
    author: meta.author ? meta.author : 'Skywa04885',
    session: session
  }, custom);
}
