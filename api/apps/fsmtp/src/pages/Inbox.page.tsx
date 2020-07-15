import React from 'react';
import Meta from '../services/Meta.service';

export default class InboxPage extends React.Component<any, any>
{
  public constructor(props: any)
  {
    super(props);
  }

  componentDidMount()
  {
    Meta.setTitle('Inbox');
  }

  public render(): any
  {
    return (
      <div className="inbox-page">
        
      </div>
    );
  }
}