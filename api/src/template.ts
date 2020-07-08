export function prepare(meta: {
  title?: string,
  description?: string,
  keywords?: string,
  author?: string
}, lang: any = {}, custom: any = {}): any
{
  return Object.assign({
    title: meta.title ? meta.title : 'No title',
    description: meta.description ? meta.description : 'No description',
    keywords: meta.keywords ? meta.keywords : 'unknown,page',
    author: meta.author ? meta.author : 'No author',
    lang: lang
  }, custom);
}
